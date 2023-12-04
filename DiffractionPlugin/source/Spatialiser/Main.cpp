
//#include "pch.h"
#include "Spatialiser/Main.h"
#include "IUnityInterface.h"
#include <utility>

#define GA_CC UNITY_INTERFACE_API
#define GA_EXPORT UNITY_INTERFACE_EXPORT

using namespace Spatialiser;

static float* buffer = nullptr;

extern "C"
{
#pragma region UnityPluginInterface

	GA_EXPORT void GA_CC UnityPluginLoad(IUnityInterfaces* unityInterfaces)
	{
		(void)unityInterfaces;
	}

	GA_EXPORT void GA_CC UnityPluginUnload()
	{

	}

#pragma endregion


	GA_EXPORT void GA_CC HRTFInitContext(int sampleRate, int bufferSize, int hrtfResamplingStep, int maxRefOrder, int hrtfMode)
	{
		Config config;
		config.sampleRate = sampleRate;
		config.bufferSize = bufferSize;
		config.hrtfResamplingStep = hrtfResamplingStep;
		config.maxRefOrder = maxRefOrder;
		switch (hrtfMode)
		{
			case 0:
			{
				config.hrtfMode = HRTFMode::quality;
				break;
			}
			case 1:
			{
				config.hrtfMode = HRTFMode::performance;
				break;
			}
			case 2:
			{
				config.hrtfMode = HRTFMode::none;
				break;
			}
		}
		Init(&config);
	}

	GA_EXPORT void GA_CC HRTFExitContext()
	{
		Exit();
	}

	GA_EXPORT void GA_CC HRTFUpdateConfig(int diffractionDepth, bool shadowOnly)
	{
		ISMConfig config;
		config.shadowOnly = shadowOnly;
		switch (diffractionDepth)
		{
			case 0:
			{
				config.diffraction = DiffractionDepth::none;
				break;
			}
			case 1:
			{
				config.diffraction = DiffractionDepth::edgeOnly;
				break;
			}
			case 2:
			{
				config.diffraction = DiffractionDepth::edSp;
				break;
			}
			default:
			{
				config.diffraction = DiffractionDepth::none;
				break;
			}
		}
		UpdateISMConfig(config);
	}


	GA_EXPORT int GA_CC HRTFInitWall(float nX, float nY, float nZ, const float* vData, int numVertices, float aL, float aML, float aM, float aMH, float aH, int reverbWallId)
	{
		Absorption absorption = Absorption(aL, aML, aM, aMH, aH);
		vec3 normal = vec3(nX, nY, nZ);
		ReverbWall reverbWall;
		switch (reverbWallId)
		{
			case 0:
			{
				reverbWall = ReverbWall::posZ;
				break;
			}
			case 1:
			{
				reverbWall = ReverbWall::negZ;
				break;
			}
			case 2:
			{
				reverbWall = ReverbWall::posX;
				break;
			}
			case 3:
			{
				reverbWall = ReverbWall::negX;
				break;
			}
			case 4:
			{
				reverbWall = ReverbWall::posY;
				break;
			}
			case 5:
			{
				reverbWall = ReverbWall::negY;
				break;
			}
			default:
			{
				reverbWall = ReverbWall::none;
				break;
			}
		}
		return (int)InitWall(normal, vData, (size_t)numVertices, absorption, reverbWall);
	}

	GA_EXPORT void GA_CC HRTFUpdateWall(int id, float nX, float nY, float nZ, const float* vData, int numVertices, float aL, float aML, float aM, float aMH, float aH, int reverbWallId)
	{
		Absorption absorption = Absorption(aL, aML, aM, aMH, aH);
		vec3 normal = vec3(nX, nY, nZ);
		ReverbWall reverbWall;
		switch (reverbWallId)
		{
		case 0:
		{
			reverbWall = ReverbWall::posZ;
			break;
		}
		case 1:
		{
			reverbWall = ReverbWall::negZ;
			break;
		}
		case 2:
		{
			reverbWall = ReverbWall::posX;
			break;
		}
		case 3:
		{
			reverbWall = ReverbWall::negX;
			break;
		}
		case 4:
		{
			reverbWall = ReverbWall::posY;
			break;
		}
		case 5:
		{
			reverbWall = ReverbWall::negY;
			break;
		}
		default:
		{
			reverbWall = ReverbWall::none;
			break;
		}
		}		
		UpdateWall((size_t)id, normal, vData, (size_t)numVertices, absorption, reverbWall);
	}

	GA_EXPORT void GA_CC HRTFRemoveWall(int id, int reverbWallId)
	{
		ReverbWall reverbWall;
		switch (reverbWallId)
		{
		case 0:
		{
			reverbWall = ReverbWall::posZ;
			break;
		}
		case 1:
		{
			reverbWall = ReverbWall::negZ;
			break;
		}
		case 2:
		{
			reverbWall = ReverbWall::posX;
			break;
		}
		case 3:
		{
			reverbWall = ReverbWall::negX;
			break;
		}
		case 4:
		{
			reverbWall = ReverbWall::posY;
			break;
		}
		case 5:
		{
			reverbWall = ReverbWall::negY;
			break;
		}
		default:
		{
			reverbWall = ReverbWall::none;
			break;
		}
		}
		RemoveWall((size_t)id, reverbWall);
	}

	GA_EXPORT void GA_CC HRTFSetFDNParameters(float volume, const float* dim, int numDimensions)
	{
		vec dimensions = vec(dim, numDimensions);
		SetFDNParameters(volume, dimensions);
	}

	GA_EXPORT bool GA_CC HRTFFilesLoaded()
	{
		return FilesLoaded();
	}

	GA_EXPORT void GA_CC HRTFUpdateListener(float posX, float posY, float posZ, float oriW, float oriX, float oriY, float oriZ)
	{
		UpdateListener(vec3(posX, posY, posZ), quaternion(oriW, oriX, oriY, oriZ));
	}

	GA_EXPORT int GA_CC HRTFInitSource()
	{
		return (int)InitSource();
	}

	GA_EXPORT void GA_CC HRTFUpdateSource(int id, float posX, float posY, float posZ, float oriW, float oriX, float oriY, float oriZ)
	{
		UpdateSource((size_t)id, vec3(posX, posY, posZ), quaternion(oriW, oriX, oriY, oriZ));
	}

	GA_EXPORT void GA_CC HRTFRemoveSource(int id)
	{
		RemoveSource((size_t)id);
	}

	GA_EXPORT void GA_CC HRTFSubmitAudio(int id, const float* data, int numFrames)
	{
		SubmitAudio((size_t)id, data, (size_t)numFrames);
	}

	GA_EXPORT bool GA_CC HRTFProcessOutput()
	{
		GetOutput(&buffer);
		if (!buffer)
		{
			Debug::Log("Process Output False1", Color::Orange);
			return false;
		}
		else if (std::isnan(*buffer))
		{
			Debug::Log("Process Output False2", Color::Orange);
			return false;
		}
		Debug::Log("Process Output True", Color::Orange);
		return true;
	}

	GA_EXPORT void GA_CC HRTFGetOutputBuffer(float** buf)
	{
		*buf = buffer;
	}
}