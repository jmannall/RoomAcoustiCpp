/**
* @brief Scene examples for profiling. Includes a main entry point for building an executable.
*/

#include <iostream>

#include "Spatialiser/Interface.h"
#include "Spatialiser/ContextOptionalArguments.h"
#include "Common/Debug.h"

#include "MoDARTLoader.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <filesystem>
#include <framework.h>

#include <random>

#include "CommandLineParser.h"

using namespace RAC::Spatialiser;
using namespace RAC::Common;
using namespace RAC::DSP;

#define DEBUG_MEMORY        ( 0 && !defined(NDEBUG) )

// Enable this to use the AMD uProf timing API
#define USE_AMD_UPROF			( 0 )
#define USE_INTEL_VTUNE			( 0 )

#if USE_AMD_UPROF
#include <AMDProfileController.h>
#pragma comment(lib, "AMDProfileController")
#pragma comment(lib, "Shell32")
#endif
#if USE_INTEL_VTUNE
#include <ittnotify.h>
#pragma comment(lib, "libittnotify")
#endif

#if DEBUG_MEMORY
struct MemoryAllocationData
{
	LONG AllocCount = 0;
	LONG ReallocCount = 0;
	LONG FreeCount = 0;
	LONG64 Size = 0;
	LONG TinyCount = 0;      // <= 16 bytes
	LONG SmallCount = 0;     // <= 64 bytes
	LONG MediumCount = 0;    // <= 256 bytes
	LONG LargeCount = 0;     // <= 1024 bytes
	LONG XLargeCount = 0;    // <= 4096 bytes
	LONG XXLargeCount = 0;   // <= 16384 BYTE
	LONG XXXLargeCount = 0;  // > 16384

	int TrackedThreadCount = 0;
	static constexpr int MaxTrackedThreads = 16;
	DWORD TrackedThreads[MaxTrackedThreads];
};

MemoryAllocationData g_MemoryAllocationData;
CRITICAL_SECTION g_MemoryDebugCriticalSection;

void InitMemoryDebug()
{
	InitializeCriticalSection(&g_MemoryDebugCriticalSection);
}

void ExitMemoryDebug()
{
	DeleteCriticalSection(&g_MemoryDebugCriticalSection);
}

void ResetMemoryAllocationMonitoring()
{
	g_MemoryAllocationData = MemoryAllocationData();
}

int AllocatorHook(
	int nAllocType,
	void* pUserData,
	size_t nSize,
	int nBlockUse,
	long lRequest,
	const unsigned char* szFileName,
	int nLine)
{
	if (nAllocType == _HOOK_ALLOC || nAllocType == _HOOK_REALLOC)
	{
		if (nAllocType == _HOOK_ALLOC)
			InterlockedIncrement(&g_MemoryAllocationData.AllocCount);
		else
			InterlockedIncrement(&g_MemoryAllocationData.ReallocCount);
		InterlockedAdd64(&g_MemoryAllocationData.Size, nSize);
		if (nSize <= 16)
			InterlockedIncrement(&g_MemoryAllocationData.TinyCount);
		else if (nSize <= 64)
			InterlockedIncrement(&g_MemoryAllocationData.SmallCount);
		else if (nSize <= 256)
			InterlockedIncrement(&g_MemoryAllocationData.MediumCount);
		else if (nSize <= 1024)
			InterlockedIncrement(&g_MemoryAllocationData.LargeCount);
		else if (nSize <= 4096)
			InterlockedIncrement(&g_MemoryAllocationData.XLargeCount);
		else if (nSize <= 16384)
			InterlockedIncrement(&g_MemoryAllocationData.XXLargeCount);
		else
			InterlockedIncrement(&g_MemoryAllocationData.XXXLargeCount);

	}
	else if (nAllocType == _HOOK_FREE)
	{
		InterlockedIncrement(&g_MemoryAllocationData.FreeCount);
	}

	// find the thread we are using; this list could theoretically change during
	// iteration
	bool found = false;
	const auto tid = GetCurrentThreadId();
	for (int index = 0; index < g_MemoryAllocationData.TrackedThreadCount; ++index)
	{
		if (g_MemoryAllocationData.TrackedThreads[index] == tid)
		{
			found = true;
			break;
		}

	}
	if (!found && g_MemoryAllocationData.TrackedThreadCount < MemoryAllocationData::MaxTrackedThreads)
	{
		EnterCriticalSection(&g_MemoryDebugCriticalSection);
		if (g_MemoryAllocationData.TrackedThreadCount < MemoryAllocationData::MaxTrackedThreads)
			g_MemoryAllocationData.TrackedThreads[g_MemoryAllocationData.TrackedThreadCount++] = tid;
		LeaveCriticalSection(&g_MemoryDebugCriticalSection);
	}

	return TRUE;
}

void StartMemoryMonitor()
{
	_CrtSetAllocHook(AllocatorHook);
}

void StopMemoryMonitor()
{
	_CrtSetAllocHook(nullptr);
}

void DumpMemory()
{
	std::cout << std::format("Memory: Alloc={}/Re={}/Free={}; Total: {} bytes (t={}/S={}/M={}/L={}/XL={}/XXL={}/XXXL={}) threads={}",
		g_MemoryAllocationData.AllocCount,
		g_MemoryAllocationData.ReallocCount,
		g_MemoryAllocationData.FreeCount,
		g_MemoryAllocationData.Size,
		g_MemoryAllocationData.TinyCount,
		g_MemoryAllocationData.SmallCount,
		g_MemoryAllocationData.MediumCount,
		g_MemoryAllocationData.LargeCount,
		g_MemoryAllocationData.XLargeCount,
		g_MemoryAllocationData.XXLargeCount,
		g_MemoryAllocationData.XXXLargeCount,
		g_MemoryAllocationData.TrackedThreadCount) << std::endl;
}

#else
inline void StartMemoryMonitor() {}
inline void StopMemoryMonitor() {}
#endif


class BaseTest
{
public:
	void Run();

protected:
	BaseTest(ProfileExecutionContext& executionContext) : executionContext(executionContext) {}

	ProfileExecutionContext& executionContext;
	Coefficients<> frequencyBands;

	int numFrames{ 512 };						// Number of frames per audio callback
	Buffer<> input = Buffer<>::Zero(numFrames);
	Buffer<> output = Buffer<>::Zero(2 * numFrames);

	virtual bool Init();
	virtual void Main() = 0;
	virtual void Exit();

	std::default_random_engine generator{ 1764 }; // Seed the generator
	std::uniform_real_distribution<float> distribution{ -0.5, 0.5 };
	inline float RandomValue() { return distribution(generator); }
};

void BaseTest::Run()
{
	executionContext.SetExecutionStage(ProfileExecutionStage::Init);
	if (!Init())
		return;

#if USE_AMD_UPROF
	amdProfileResume();
#endif
#if USE_INTEL_VTUNE
	__itt_resume();
#endif
	executionContext.SetExecutionStage(ProfileExecutionStage::Main);
	StartMemoryMonitor();
	Main();
	StopMemoryMonitor();
#if USE_INTEL_VTUNE
	__itt_pause();
#endif
#if USE_AMD_UPROF
	amdProfilePause();
#endif

	executionContext.SetExecutionStage(ProfileExecutionStage::Exit);
	Exit();
}

void DebugCallback(const char* message, int type, int size)
{
	std::cout << "[" << ToString((DebugType)type) << "] " << message << std::endl;
}

bool BaseTest::Init()
{
	RegisterDebugCallback(DebugCallback);

	int fs{ 48000 };							// Sample rate
	int numReverbSources{ 12 };					// Number of output channels for late reverberation
	int fdnSize{ 12 };							// Size of the FDN (number of delay lines)
	Real lerpFactor = 2.0; 				        // Interpolation factor
	Real Q{ 0.98 };								// Q factor for the GraphicEQ
	frequencyBands = Coefficients<>(std::vector<Real>({ 125.0, 250.0, 500.0, 1e3, 2e3, 4e3, 8e3 }));				// Frequency band center frequencies

	DSPData configData = DSPData(fs, numFrames, numReverbSources, fdnSize, lerpFactor, Q, frequencyBands);
	ContextOptionalArguments optionalArguments =
	{
		.logPrefix = executionContext.logPrefix,
		.desiredAudioThreads = executionContext.desiredAudioThreads
	};
	::Init(configData, optionalArguments);

	int hrtfSamplingStep = 5;
	static std::vector<std::string> hrtfFiles = { "HRTF/Kemar_DTF_ITD_48000_3dti-hrtf.3dti-hrtf", "HRTF/NearFieldCompensation_ILD_48000.3dti-ild", "HRTF/HRTF_ILD_48000.3dti-ild" };
	bool success = LoadSpatialisationFiles(hrtfSamplingStep, hrtfFiles);
	if (!success)
	{
		std::cout << "Error loading spatialisation files!" << std::endl;
		UpdateSpatialisationMode(SpatialisationMode::none);
	}
	else
	{
		UpdateSpatialisationMode(executionContext.useQualityHrtfs ? SpatialisationMode::quality : SpatialisationMode::performance);
	}

	DirectSound dir = DirectSound::check;
	const int reflOrder = executionContext.reflectionOrder;
	const int shadowOrder = executionContext.shadowOrder;
	int specularOrder = 1;
	Real minEdgeLength = (Real)0.0;
	Real maxPathLength = 1e4; // No limit on path length
	EarlyReverbData earlyReverbData(dir, reflOrder, shadowOrder, specularOrder, minEdgeLength, maxPathLength);

	DiffractionModel diffractionModel = DiffractionModel::nnSmall;

	InitEarlyReverb(true, earlyReverbData, diffractionModel);
	return true;
}

void BaseTest::Exit()
{
	UnregisterDebugCallback();
	::Exit();
}


class ProfileShoeboxTest : public BaseTest
{
public:
	explicit ProfileShoeboxTest(ProfileExecutionContext& executionContext) : BaseTest(executionContext) {}

protected:
	virtual bool Init() override;
	virtual void Main() override;
	virtual void Exit() override;

	Vec3 listenerPos = Vec3((Real)0.0, (Real)2.0, (Real)0.0);
	Vec4 listenerOri = Vec4((Real)1.0, (Real)0.0, (Real)0.0, (Real)0.0);

	Vec3 sourcePos = Vec3((Real)1.0, (Real)2.0, (Real)3.0);
	Vec4 sourceOri = Vec4((Real)1.0, (Real)0.0, (Real)0.0, (Real)0.0);
	size_t materialId;
	std::vector<size_t> wallIds;
	int id;

	static std::vector<size_t> CreateShoeboxRoom(Vec3 pos, size_t materialId);

};


bool ProfileShoeboxTest::Init()
{
	if (!BaseTest::Init())
		return false;

	// Create shoebox
	Vec3 pos(7.0, 3.0, 4.0);
	Coefficients<> absorption(std::vector<Real>({ 0.03, 0.03, 0.04, 0.06, 0.09, 0.1, 0.12 }));
	materialId = InitMaterial(absorption);
	wallIds = CreateShoeboxRoom(pos, materialId);

	FDNMatrix matrix = FDNMatrix::randomOrthogonal;

	Real volume = pos.Sum();
	Coefficients<> t60(std::vector<Real>({ 0.8, 0.7, 0.65, 0.63, 0.6, 0.55, 0.48 }));
	ReverbFormula formula = ReverbFormula::Custom;
	Vec<> dimensions{ { pos.x(), pos.y(), pos.z()} };
	RoomData roomData(volume, t60, formula, dimensions);

	LateReverbData lateReverbData(true, executionContext.numRays, matrix);

	InitSingleFDN(roomData, lateReverbData);

	input[0] = 1.0;
	// Stereo output buffer

	UpdateListener(listenerPos, listenerOri);

	id = InitSource();
	if (id < 0)
	{
		std::cout << "Error initialising source!" << std::endl;
		return false;
	}
	UpdateSourceDirectivity(static_cast<size_t>(id), SourceDirectivity::genelec8020c);

	UpdateSource(static_cast<size_t>(id), sourcePos, sourceOri);

	// Only run to ensure background processes have run at least once
	// No point profiling audio before image edge model and late reverb are ready
	RecordImpulseResponse(sourcePos, sourceOri, output);
	return true;
}

void ProfileShoeboxTest::Main()
{
	for (int innerIteration = 0; innerIteration < executionContext.innerIterations; ++innerIteration)
	{
		SubmitAudio(static_cast<size_t>(id), input);
		GetOutput(output);

		if (executionContext.staticScene)
			continue;

		Vec3 position = listenerPos;
		position.x() += RandomValue();
		position.z() += RandomValue();
		UpdateListener(position, listenerOri);

		position = sourcePos;
		position.x() += RandomValue();
		position.z() += RandomValue();
		UpdateSource(static_cast<size_t>(id), position, sourceOri);
	}
}

void ProfileShoeboxTest::Exit()
{
	for (size_t wallID : wallIds)
		RemoveWall(wallID);
	RemoveMaterial(materialId);
	RemoveSource(static_cast<size_t>(id));
	BaseTest::Exit();
}


std::vector<size_t> ProfileShoeboxTest::CreateShoeboxRoom(Vec3 pos, size_t materialId)
{
	Real posX = pos.x();
	Real posY = pos.y();
	Real posZ = pos.z();

	std::vector<size_t> wallIDs(12);
	wallIDs[0] = InitWall({ Vec3((Real)0.0, posY, (Real)0.0),
			Vec3(posX, posY, (Real)0.0),
			Vec3(posX, posY, posZ) }, materialId);
	wallIDs[1] = InitWall({ Vec3((Real)0.0, posY, (Real)0.0),
			Vec3(posX, posY, posZ),
			Vec3((Real)0.0, posY, posZ) }, materialId);
	wallIDs[2] = InitWall({ Vec3(posX, (Real)0.0, (Real)0.0),
			Vec3((Real)0.0, (Real)0.0, (Real)0.0),
			Vec3((Real)0.0, (Real)0.0, posZ) }, materialId);
	wallIDs[3] = InitWall({ Vec3(posX, (Real)0.0, (Real)0.0),
			Vec3((Real)0.0, (Real)0.0, posZ),
			Vec3(posX, (Real)0.0, posZ) }, materialId);
	wallIDs[4] = InitWall({ Vec3(posX, (Real)0.0, posZ),
			Vec3(posX, posY, posZ),
			Vec3(posX, posY, (Real)0.0) }, materialId);
	wallIDs[5] = InitWall({ Vec3(posX, (Real)0.0, posZ),
			Vec3(posX, posY, (Real)0.0),
			Vec3(posX, (Real)0.0, (Real)0.0) }, materialId);
	wallIDs[6] = InitWall({ Vec3((Real)0.0, (Real)0.0, (Real)0.0),
			Vec3((Real)0.0, posY, (Real)0.0),
			Vec3((Real)0.0, posY, posZ) }, materialId);
	wallIDs[7] = InitWall({ Vec3((Real)0.0, (Real)0.0, (Real)0.0),
			Vec3((Real)0.0, posY, posZ),
			Vec3((Real)0.0, (Real)0.0, posZ) }, materialId);
	wallIDs[8] = InitWall({ Vec3((Real)0.0, (Real)0.0, (Real)0.0),
			Vec3(posX, (Real)0.0, (Real)0.0),
			Vec3(posX, posY, (Real)0.0) }, materialId);
	wallIDs[9] = InitWall({ Vec3((Real)0.0, (Real)0.0, (Real)0.0),
			Vec3(posX, posY, (Real)0.0),
			Vec3((Real)0.0, posY, (Real)0.0) }, materialId);
	wallIDs[10] = InitWall({ Vec3((Real)0.0, posY, posZ),
			Vec3(posX, posY, posZ),
			Vec3(posX, (Real)0.0, posZ) }, materialId);
	wallIDs[11] = InitWall({ Vec3((Real)0.0, posY, posZ),
			Vec3(posX, (Real)0.0, posZ),
			Vec3((Real)0.0, (Real)0.0, posZ) }, materialId);
	UpdatePlanesAndEdges();
	return wallIDs;
}

void ProfileShoebox(ProfileExecutionContext& executionContext)
{
	ProfileShoeboxTest test(executionContext);
	test.Run();
}

class ProfileMoDARTTest : public BaseTest
{
public:
	ProfileMoDARTTest(ProfileExecutionContext& executionContext) : BaseTest(executionContext) {}

protected:
	virtual bool Init() override;
	virtual void Main() override;
	virtual void Exit() override;

	Vec3 listenerPos = Vec3((Real)2.0, (Real)1.0, (Real)6.8);
	Vec4 listenerOri = Vec4((Real)1.0, (Real)0.0, (Real)0.0, (Real)0.0);

	Vec3 sourcePos = Vec3((Real)2.0, (Real)1.5, (Real)2.0);
	Vec4 sourceOri = Vec4((Real)1.0, (Real)0.0, (Real)0.0, (Real)0.0);
	int id;
};

bool ProfileMoDARTTest::Init()
{
	if (!BaseTest::Init())
		return false;

	// Load MoDART scene
	FDNMatrix matrix = FDNMatrix::randomOrthogonal;
	LateReverbData lateReverbData(true, executionContext.numRays, matrix);

	std::string modartPath = "MoDART";
	if (!LoadMoDARTScene(modartPath, frequencyBands, lateReverbData))
	{
		std::cout << "Error loading MoDART scene!" << std::endl;
		Exit();
		return false;
	}

	input[0] = 1.0;
	// Stereo output buffer

	UpdateListener(listenerPos, listenerOri);

	id = InitSource();
	if (id < 0)
	{
		std::cout << "Error initialising source!" << std::endl;
		return false;
	}
	UpdateSourceDirectivity(static_cast<size_t>(id), SourceDirectivity::genelec8020c);

	UpdateSource(static_cast<size_t>(id), sourcePos, sourceOri);

	// Only run to ensure background processes have run at least once
	// No point profiling audio before image edge model and late reverb are ready
	RecordImpulseResponse(sourcePos, sourceOri, output);

	return true;
}

void ProfileMoDARTTest::Main()
{
	for (int innerIteration = 0; innerIteration < executionContext.innerIterations; ++innerIteration)
	{
		SubmitAudio(static_cast<size_t>(id), input);
		GetOutput(output);

		if (executionContext.staticScene)
			continue;

		Vec3 position = listenerPos;
		position.x() += RandomValue();
		position.z() += RandomValue();
		UpdateListener(position, listenerOri);

		position = sourcePos;
		position.x() += RandomValue();
		position.z() += RandomValue();
		UpdateSource(static_cast<size_t>(id), position, sourceOri);
	}
}

void ProfileMoDARTTest::Exit()
{
	RemoveSource(static_cast<size_t>(id));
	BaseTest::Exit();
}

void ProfileMoDART(ProfileExecutionContext& executionContext)
{
	ProfileMoDARTTest test(executionContext);
	test.Run();
}

class ProfileMoDARTManySourcesTest : public BaseTest
{
public:
	ProfileMoDARTManySourcesTest(ProfileExecutionContext& executionContext) : BaseTest(executionContext) {}

protected:
	virtual bool Init() override;
	virtual void Main() override;
	virtual void Exit() override;

	Vec3 listenerPos = Vec3((Real)3.2, (Real)1.5, (Real)3.76);
	Vec4 listenerOri = Vec4((Real)1.0, (Real)0.0, (Real)0.0, (Real)0.0);

	std::vector<Vec3> sourcePos = { Vec3((Real)3.08, (Real)1.0, (Real)1.078),
		Vec3((Real)1.35, (Real)1.5, (Real)1.07),
		Vec3((Real)2.22, (Real)1.5, (Real)1.988),
		Vec3((Real)8.54, (Real)2.5, (Real)2.568),
		Vec3((Real)1.401, (Real)1.5, (Real)7.215),
		Vec3((Real)7.01, (Real)1.75, (Real)8.56),
		Vec3((Real)4.35, (Real)1.75, (Real)4.701), };
	Vec4 sourceOri = Vec4((Real)1.0, (Real)0.0, (Real)0.0, (Real)0.0);
	std::vector<int> ids = { -1, -1, -1, -1, -1, -1, -1 };
	int numSources = 7;

};

bool ProfileMoDARTManySourcesTest::Init()
{
	if (!BaseTest::Init())
		return false;

	// Load MoDART scene
	FDNMatrix matrix = FDNMatrix::householder;
	LateReverbData lateReverbData(true, executionContext.numRays, matrix);

	std::string modartPath = "MoDART";
	if (!LoadMoDARTScene(modartPath, frequencyBands, lateReverbData))
	{
		std::cout << "Error loading MoDART scene!" << std::endl;
		Exit();
		return false;
	}

	input[0] = 1.0;
	// Stereo output buffer
	UpdateListener(listenerPos, listenerOri);

	for (int i = 0; i < numSources; ++i)
	{
		ids[i] = InitSource();
		if (ids[i] < 0)
		{
			std::cout << "Error initialising source!" << std::endl;
			return false;
		}
		UpdateSourceDirectivity(static_cast<size_t>(ids[i]), SourceDirectivity::genelec8020c);

		UpdateSource(static_cast<size_t>(ids[i]), sourcePos[i], sourceOri);
	}

	// Only run to ensure background processes have run at least once
	// No point profiling audio before image edge model and late reverb are ready
	RecordImpulseResponse(sourcePos[0], sourceOri, output);

	return true;
}

void ProfileMoDARTManySourcesTest::Main()
{
	for (int innerIteration = 0; innerIteration < executionContext.innerIterations; ++innerIteration)
	{
		for (auto id : ids)
			SubmitAudio(static_cast<size_t>(id), input);
		GetOutput(output);

		if (executionContext.staticScene)
			continue;

		Vec3 position = listenerPos;
		position.x() += RandomValue();
		position.z() += RandomValue();
		UpdateListener(position, listenerOri);
		for (int i = 0; i < numSources; ++i)
		{
			position = sourcePos[i];
			position.x() += RandomValue();
			position.z() += RandomValue();
			UpdateSource(static_cast<size_t>(ids[i]), position, sourceOri);
		}
	}
}

void ProfileMoDARTManySourcesTest::Exit()
{
	for (auto id : ids)
		RemoveSource(static_cast<size_t>(id));
	BaseTest::Exit();
}

void ProfileMoDARTManySources(ProfileExecutionContext& executionContext)
{
	ProfileMoDARTManySourcesTest test(executionContext);
	test.Run();
}

// Common::CTimeMeasure requires using the whole profile to properly work, so just
// create a simple class to manage the time that we want

bool ChangeToProfilingDirectory(const std::string& userPath)
{
	namespace fs = std::filesystem;

	if (!userPath.empty())
	{
		if (fs::exists(userPath))
		{
			std::cerr << "Profiling data given but doesn't exist (" << userPath << ")" << std::endl;
			return false;
		}

		fs::current_path(userPath);
		return true;
	}
	else
	{
		fs::path path = fs::current_path();
		for (;; )
		{
			fs::path profileDataCandidate = path / "ProfilingData";
			if (fs::exists(profileDataCandidate))
			{
				fs::current_path(profileDataCandidate);
				return true;
			}

			if (!path.has_parent_path() || path == path.root_path())
			{
				std::cerr << "Searched up but cannot find ProfilingData" << std::endl;
				return false;
			}
			path = path.parent_path();
		}
	}

}

int main(int argc, const char* argv[])
{
	SetThreadDescription(GetCurrentThread(), TEXT("Main"));

#if DEBUG_MEMORY
	InitMemoryDebug();
#endif

	CommandLineParser commandLineParser(argc, argv);
	commandLineParser.RegisterProfileTest("Shoebox", ProfileShoebox);
	commandLineParser.RegisterProfileTest("MoDART", ProfileMoDART);
	commandLineParser.RegisterProfileTest("MoDARTManySources", ProfileMoDARTManySources);
	if (!commandLineParser.Parse())
		return -1;

#if _DEBUG
	if (commandLineParser.GetDebugFlag())
	{
		std::cout << "Enabling debug flag" << std::endl;
		// this only works in debug
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	}
#endif

	if (!ChangeToProfilingDirectory(commandLineParser.GetProfileDataDirectory()))
		return -1;

	const auto& plan = commandLineParser.GetPlan();
	const int iterations = commandLineParser.GetTestIterations();
	std::cout << "Running " << plan.size() << " tests " << iterations << " times." << std::endl;

	std::ofstream runLog(commandLineParser.GetLogPrefix() + "_run.txt");
	runLog << "Test,Type,Iterations,TotalInitTime,TotalMainTime,TotalExitTime,IterationInitTime,IterationMainTime,IterationExitTime,InnerLoopTime" << std::endl;

	for (size_t planIndex = 0; planIndex < plan.size(); ++planIndex)
	{
		ProfileExecutionContext executionContext =
		{
			.name = plan[planIndex].name,
			.logPrefix = commandLineParser.GetDetailedLogs() ? commandLineParser.GetLogPrefix() : "",
			.totalTestIterations = iterations,
			.innerIterations = commandLineParser.GetInnerIterations(),
			.numRays = commandLineParser.GetNumRays(),
			.reflectionOrder = commandLineParser.GetReflectionOrder(),
			.shadowOrder = commandLineParser.GetShadowOrder(),
			.staticScene = commandLineParser.GetStaticSceneFlag(),
			.useQualityHrtfs = commandLineParser.GetUseQualityHRTFs(),
			.desiredAudioThreads = commandLineParser.GetDesiredAudioThreads()
		};

		std::cout << "Profiling: " << executionContext.name << std::endl;
#if DEBUG_MEMORY
		ResetMemoryAllocationMonitoring();
#endif
		for (int iteration = 0; iteration < iterations; ++iteration)
		{
			std::cout << "Running iteration " << (iteration + 1) << "/" << iterations << std::endl;
			executionContext.currentTestIteration = iteration;
			plan[planIndex].function(executionContext);
			executionContext.CompleteExecution();
		}

#if DEBUG_MEMORY
		DumpMemory();
#endif

		// log it
		const int totalInnerIterations = iterations * executionContext.innerIterations;
		runLog << executionContext.name << ","
			<< (MATRIX_LIBRARY == EIGEN_FLAG ? "Eigen," : "Custom,")
			<< iterations << ","
			<< executionContext.TotalTime << ","
			<< executionContext.InitTime << ","
			<< executionContext.MainTime << ","
			<< executionContext.ExitTime << ","
			<< (executionContext.TotalTime / iterations) << ","
			<< (executionContext.InitTime / iterations) << ","
			<< (executionContext.MainTime / iterations) << ","
			<< (executionContext.ExitTime / iterations) << ","
			<< (executionContext.MainTime / totalInnerIterations) <<
			std::endl;

		std::cout << std::format("Test time: {:.1f}ms ({:.1f}ms/{:.1f}ms/{:.1f}ms); Per-test: {:.1f}ms ({:.1f}ms/{:.1f}ms/{:.1f}ms); Per-iteration: {:.2f}ms)",
			executionContext.TotalTime,
			executionContext.InitTime,
			executionContext.MainTime,
			executionContext.ExitTime,
			executionContext.TotalTime / iterations,
			executionContext.InitTime / iterations,
			executionContext.MainTime / iterations,
			executionContext.ExitTime / iterations,
			executionContext.MainTime / totalInnerIterations)
			<< std::endl;
	}

	std::cout << "Done!" << std::endl;

#if DEBUG_MEMORY
	ExitMemoryDebug();
#endif
}