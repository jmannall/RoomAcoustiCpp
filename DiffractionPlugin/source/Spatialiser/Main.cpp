/*
*
*  \Defines linakage between the C# unity code and C++ code
* 
*/

#include "IUnityInterface.h"
#include "Spatialiser/Main.h"

#define UI_API UNITY_INTERFACE_API
#define UI_EXPORT UNITY_INTERFACE_EXPORT

using namespace Spatialiser;

// Pointer to return buffer
static float* buffer = nullptr;

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

	UI_EXPORT void UI_API SPATInit(int fs, int numFrames, int numChannels, int numFDNChannels, float lerpFactor, int hrtfResamplingStep, int hrtfMode)
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
		Init(&config);
	}

	UI_EXPORT void UI_API SPATExit()
	{
		Exit();
	}

	UI_EXPORT bool UI_API SPATFilesLoaded()
	{
		return FilesLoaded();
	}

	// Image Source Model

	UI_EXPORT void UI_API SPATUpdateISMConfig(int order, bool dir, bool ref, bool diff, bool refDiff, bool spDiff)
	{
		UpdateISMConfig(ISMConfig(order, dir, ref, diff, refDiff, spDiff));
	}

	// Reverb

	UI_EXPORT void UI_API SPATSetFDNParameters(float volume, const float* dim, int numDimensions)
	{
		SetFDNParameters(volume, vec(dim, numDimensions));
	}

	// Listener

	UI_EXPORT void UI_API SPATUpdateListener(float posX, float posY, float posZ, float oriW, float oriX, float oriY, float oriZ)
	{
		UpdateListener(vec3(posX, posY, posZ), quaternion(oriW, oriX, oriY, oriZ));
	}

	// Source

	UI_EXPORT int UI_API SPATInitSource()
	{
		return (int)InitSource();
	}

	UI_EXPORT void UI_API SPATUpdateSource(int id, float posX, float posY, float posZ, float oriW, float oriX, float oriY, float oriZ)
	{
		UpdateSource((size_t)id, vec3(posX, posY, posZ), quaternion(oriW, oriX, oriY, oriZ));
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

	UI_EXPORT int UI_API SPATInitWall(float nX, float nY, float nZ, const float* vData, int numVertices, float aL, float aML, float aM, float aMH, float aH, int reverbWallId)
	{
		ReverbWall reverbWall = ReturnReverbWall(reverbWallId);
		Absorption absorption = Absorption(aL, aML, aM, aMH, aH);

		return (int)InitWall(vec3(nX, nY, nZ), vData, (size_t)numVertices, absorption, reverbWall);
	}

	UI_EXPORT void UI_API SPATUpdateWall(int id, float nX, float nY, float nZ, const float* vData, int numVertices, float aL, float aML, float aM, float aMH, float aH, int reverbWallId)
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

	UI_EXPORT void UI_API SPATSubmitAudio(int id, const float* data, int numFrames)
	{
		SubmitAudio((size_t)id, data, (size_t)numFrames);
	}

	UI_EXPORT bool UI_API SPATProcessOutput()
	{
		GetOutput(&buffer);
		if (!buffer)
		{
			Debug::Log("Process Output Failed", Color::Orange);
			return false;
		}
		else if (std::isnan(*buffer))
		{
			Debug::Log("Process Output is NaN", Color::Orange);
			return false;
		}
		Debug::Log("Process Output Success", Color::Orange);
		return true;
	}

	UI_EXPORT void UI_API SPATGetOutputBuffer(float** buf)
	{
		*buf = buffer;
	}

#pragma endregion
}