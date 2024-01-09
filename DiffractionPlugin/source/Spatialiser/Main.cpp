/*
*
*  \Defines linakage between the C# unity code and C++ code
* 
*/

#include "Unity/IUnityInterface.h"
#include "Spatialiser/Main.h"

#define UI_API UNITY_INTERFACE_API
#define UI_EXPORT UNITY_INTERFACE_EXPORT

using namespace UIE::Spatialiser;
using namespace UIE::Common;
using namespace UIE::Unity;

// Pointer to return buffer
static Real* buffer = nullptr;

extern "C"
{
	//////////////////// Unity Plugin Interface ////////////////////

	UI_EXPORT void UI_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
	{
		(void)unityInterfaces;
	}

	UI_EXPORT void UI_API UnityPluginUnload()
	{

	}

	//////////////////// Spatialiser ////////////////////

	// Load and Destroy

	UI_EXPORT bool UI_API SPATInit(int fs, int numFrames, int numChannels, int numFDNChannels, Real lerpFactor, int hrtfResamplingStep, int hrtfMode)
	{
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
		Config config = Config(fs, numFrames, numChannels, numFDNChannels, lerpFactor, hrtfResamplingStep, mode);
		return Init(&config);
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

	UI_EXPORT void UI_API SPATSetFDNParameters(Real volume, const Real* dim, int numDimensions)
	{
		SetFDNParameters(volume, vec(dim, numDimensions));
	}

	// Listener

	UI_EXPORT void UI_API SPATUpdateListener(Real posX, Real posY, Real posZ, Real oriW, Real oriX, Real oriY, Real oriZ)
	{
		UpdateListener(vec3(posX, posY, posZ), vec4(oriW, oriX, oriY, oriZ));
	}

	// Source

	UI_EXPORT int UI_API SPATInitSource()
	{
		return InitSource();
	}

	UI_EXPORT void UI_API SPATUpdateSource(int id, Real posX, Real posY, Real posZ, Real oriW, Real oriX, Real oriY, Real oriZ)
	{
		UpdateSource((size_t)id, vec3(posX, posY, posZ), vec4(oriW, oriX, oriY, oriZ));
	}

	UI_EXPORT void UI_API SPATRemoveSource(int id)
	{
		RemoveSource((size_t)id);
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

	UI_EXPORT int UI_API SPATInitWall(Real nX, Real nY, Real nZ, const Real* vData, int numVertices, Real aL, Real aML, Real aM, Real aMH, Real aH, int reverbWallId)
	{
		ReverbWall reverbWall = ReturnReverbWall(reverbWallId);
		Absorption absorption = Absorption(aL, aML, aM, aMH, aH);

		return InitWall(vec3(nX, nY, nZ), vData, (size_t)numVertices, absorption, reverbWall);
	}

	UI_EXPORT void UI_API SPATUpdateWall(int id, Real nX, Real nY, Real nZ, const Real* vData, int numVertices, Real aL, Real aML, Real aM, Real aMH, Real aH, int reverbWallId)
	{
		ReverbWall reverbWall = ReturnReverbWall(reverbWallId);
		Absorption absorption = Absorption(aL, aML, aM, aMH, aH);

		UpdateWall((size_t)id, vec3(nX, nY, nZ), vData, (size_t)numVertices, absorption, reverbWall);
	}

	UI_EXPORT void UI_API SPATRemoveWall(int id, int reverbWallId)
	{
		ReverbWall reverbWall = ReturnReverbWall(reverbWallId);

		RemoveWall((size_t)id, reverbWall);
	}

	// Audio

	UI_EXPORT void UI_API SPATSubmitAudio(int id, const Real* data)
	{
		SubmitAudio((size_t)id, data);
	}

	UI_EXPORT bool UI_API SPATProcessOutput()
	{
		GetOutput(&buffer);
		if (!buffer)
		{
			Debug::Log("Process Output Failed", Colour::Orange);
			return false;
		}
		else if (std::isnan(*buffer))
		{
			Debug::Log("Process Output is NaN", Colour::Orange);
			return false;
		}
		Debug::Log("Process Output Success", Colour::Orange);
		return true;
	}

	UI_EXPORT void UI_API SPATGetOutputBuffer(Real** buf)
	{
		*buf = buffer;
	}

#pragma endregion
}