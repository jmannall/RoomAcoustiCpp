/*
*
*  \Defines linakage between the C# unity code and C++ code
* 
*/

// Unity headers
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityProfiler.h"
#include "Unity/Debug.h"
#include "Unity/Profiler.h"

// Common headers
#include "Common/AudioManager.h" 

// Spatialiser headers
// #include "Spatialiser/Main.h"
#include "Spatialiser/Types.h"
#include "Spatialiser/Interface.h"

#define UI_API UNITY_INTERFACE_API
#define UI_EXPORT UNITY_INTERFACE_EXPORT

using namespace UIE::Spatialiser;
using namespace UIE::Common;
using namespace UIE::Unity;

// Pointer to return buffer
static float* buffer = nullptr;

extern "C"
{
	//////////////////// Unity Plugin Interface ////////////////////

	void UI_EXPORT UI_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
	{
		/*unityProfiler = unityInterfaces->Get<IUnityProfiler>();
		if (unityProfiler == NULL)
			return;
		isDevelopmentBuild = unityProfiler->IsAvailable() != 0;
		unityProfiler->CreateMarker(&spatialiserMarker, "Spatialiser", kUnityProfilerCategoryOther, kUnityProfilerMarkerFlagDefault, 0);*/
		(void)unityInterfaces;
	}

	void UI_EXPORT UI_API UnityPluginUnload()
	{
		//unityProfiler = NULL;
	}

	//////////////////// Spatialiser ////////////////////

	// Load and Destroy

	UI_EXPORT bool UI_API SPATInit(int fs, int numFrames, int numChannels, int numFDNChannels, float lerpFactor, int hrtfResamplingStep, int hrtfMode, const char** paths)
	{
		std::vector<std::string> filePaths = { string(*(paths)), string(*(paths + 1)), string(*(paths + 2)) };
		HRTFMode mode;
		switch (hrtfMode)
		{
			case 0:
			{ mode = HRTFMode::quality; break; }
			case 1:
			{ mode = HRTFMode::performance; break; }
			case 2:
			{ mode = HRTFMode::none; break; }
			default:
			{ mode = HRTFMode::performance; break; }
		}

		Config config = Config(fs, numFrames, numChannels, numFDNChannels, static_cast<Real>(lerpFactor), hrtfResamplingStep, mode);
		return Init(&config, filePaths);
	}

	UI_EXPORT void UI_API SPATExit()
	{
		Exit();
	}

	// Image Source Model

	UI_EXPORT void UI_API SPATUpdateISMConfig(int order, bool dir, bool ref, bool diff, bool refDiff, bool rev, bool spDiff)
	{
		UpdateISMConfig(ISMConfig(order, dir, ref, diff, refDiff, rev, spDiff));
	}

	// Reverb

	UI_EXPORT void UI_API SPATSetFDNParameters(float volume, const float* dim, int numDimensions)
	{
		Buffer in = Buffer(numDimensions);
		for (int i = 0; i < numDimensions; i++)
			in[i] = static_cast<Real>(dim[i]);

		SetFDNParameters(static_cast<Real>(volume), vec(&in[0], numDimensions));
	}

	// Listener

	UI_EXPORT void UI_API SPATUpdateListener(float posX, float posY, float posZ, float oriW, float oriX, float oriY, float oriZ)
	{
		UpdateListener(vec3(posX, posY, posZ), vec4(oriW, oriX, oriY, oriZ));
	}

	// Source

	UI_EXPORT int UI_API SPATInitSource()
	{
		return InitSource();
	}

	UI_EXPORT void UI_API SPATUpdateSource(int id, float posX, float posY, float posZ, float oriW, float oriX, float oriY, float oriZ)
	{
		UpdateSource(static_cast<size_t>(id), vec3(posX, posY, posZ), vec4(oriW, oriX, oriY, oriZ));
	}

	UI_EXPORT void UI_API SPATRemoveSource(int id)
	{
		RemoveSource(static_cast<size_t>(id));
	}

	// Walls

	ReverbWall ReturnReverbWall(int id)
	{
		switch (id)
		{
		case 0:
		{ return ReverbWall::posZ; break; }
		case 1:
		{ return ReverbWall::negZ; break; }
		case 2:
		{ return ReverbWall::posX; break; }
		case 3:
		{ return ReverbWall::negX; break; }
		case 4:
		{ return ReverbWall::posY; break; }
		case 5:
		{ return ReverbWall::negY; break; }
		default:
		{ return ReverbWall::none; break; }
		}
	}

	UI_EXPORT int UI_API SPATInitWall(float nX, float nY, float nZ, const float* vData, int numVertices, float aL, float aML, float aM, float aMH, float aH, int reverbWallId)
	{
		ReverbWall reverbWall = ReturnReverbWall(reverbWallId);
		Absorption absorption = Absorption(aL, aML, aM, aMH, aH);

		int numCoords = 3 * numVertices;
		Buffer in = Buffer(numCoords);
		for (int i = 0; i < numCoords; i++)
			in[i] = static_cast<Real>(vData[i]);

		return InitWall(vec3(nX, nY, nZ), &in[0], static_cast<size_t>(numVertices), absorption, reverbWall);
	}

	UI_EXPORT void UI_API SPATUpdateWall(int id, float nX, float nY, float nZ, const float* vData, int numVertices, float aL, float aML, float aM, float aMH, float aH, int reverbWallId)
	{
		ReverbWall reverbWall = ReturnReverbWall(reverbWallId);
		Absorption absorption = Absorption(aL, aML, aM, aMH, aH);

		int numCoords = 3 * numVertices;
		Buffer in = Buffer(numCoords);
		for (int i = 0; i < numCoords; i++)
			in[i] = static_cast<Real>(vData[i]);

		UpdateWall(static_cast<size_t>(id), vec3(nX, nY, nZ), &in[0], static_cast<size_t>(numVertices), absorption, reverbWall);
	}

	UI_EXPORT void UI_API SPATFreeWallId(int id)
	{
		FreeWallId(static_cast<size_t>(id));
	}

	UI_EXPORT void UI_API SPATRemoveWall(int id, int reverbWallId)
	{
		ReverbWall reverbWall = ReturnReverbWall(reverbWallId);

		RemoveWall(static_cast<size_t>(id), reverbWall);
	}

	// Audio

	UI_EXPORT void UI_API SPATSubmitAudio(int id, const float* data)
	{
		SubmitAudio(static_cast<size_t>(id), data);
	}

	UI_EXPORT bool UI_API SPATProcessOutput()
	{
		GetOutput(&buffer);
		if (!buffer)
		{
#ifdef DEBUG_AUDIO_THREAD
	Debug::Log("Process Output Failed", Colour::Orange);
#endif
			return false;
		}
		else if (std::isnan(*buffer))
		{
#ifdef DEBUG_AUDIO_THREAD
	Debug::Log("Process Output is NaN", Colour::Orange);
#endif
			return false;
		}
#ifdef DEBUG_AUDIO_THREAD
	Debug::Log("Process Output Success", Colour::Orange);
#endif
		return true;
	}

	UI_EXPORT void UI_API SPATGetOutputBuffer(float** buf)
	{
		*buf = buffer;
	}
}