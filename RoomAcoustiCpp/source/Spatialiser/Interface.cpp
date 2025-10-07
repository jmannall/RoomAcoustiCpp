/*
* @brief Interface between the API and Spatialiser global context
* 
*/

// Spatialiser headers
#include "Spatialiser/Interface.h"
#include "Spatialiser/Context.h"

// Unity headers
#include "Unity/Debug.h"

namespace RAC
{
	using namespace Unity;
	namespace Spatialiser
	{
		////////////////////////////////////////
		// Context Singleton
		static std::shared_ptr<Context> context = nullptr;

		////////////////////////////////////////

		static std::shared_ptr<Context> GetContext() { return context; }

		////////////////////////////////////////

		bool Init(const DSPData& data)
		{
			if (context) // Delete any existing context
			{
#ifdef DEBUG_INIT
	Debug::Log("Delete Existing Context", Colour::Red);
#endif
				Exit();
			}
#ifdef DEBUG_INIT
	Debug::Log("Create New Context", Colour::Green);
#endif
			context = std::make_shared<Context>(data);
			return context->IsRunning();
		}

		////////////////////////////////////////

		void Exit()
		{
			if (context)
			{
				context.reset();
				context = nullptr;
			}
		}

		////////////////////////////////////////

		bool LoadSpatialisationFiles(const int hrtfResamplingStep, const std::vector<std::string>& filePaths)
		{
			auto context = GetContext();
			if (context)
				return context->LoadSpatialisationFiles(hrtfResamplingStep, filePaths);
			else
				return false;
		}

		////////////////////////////////////////

		void SetHeadphoneEQ(const Buffer<>& leftIR, const Buffer<>& rightIR)
		{
			auto context = GetContext();
			if (context)
				context->SetHeadphoneEQ(leftIR, rightIR);
		}

		////////////////////////////////////////

		void UpdateSpatialisationMode(const SpatialisationMode mode)
		{
			auto context = GetContext();
			if (context)
				context->UpdateSpatialisationMode(mode);
		}

		////////////////////////////////////////

		void EnableEarlyReverb(const bool enable)
		{
			auto context = GetContext();
			if (context)
				context->EnableEarlyReverb(enable);
		}

		////////////////////////////////////////

		void UpdateEarlyConfig(const EarlyReverbData& data)
		{
			auto context = GetContext();
			if (context)
				context->UpdateEarlyConfig(data);
		}

		////////////////////////////////////////

		void EnableLateReverb(const bool enable)
		{
			auto context = GetContext();
			if (context)
				context->EnableLateReverb(enable);
		}

		////////////////////////////////////////

		void UpdateLateReverbNumberOfRays(const int numRays)
		{
			auto context = GetContext();
			if (context)
				context->UpdateLateReverbNumberOfRays(numRays);

		}
		////////////////////////////////////////

		void UpdateMoDARTDelay(const Real delay)
		{
			auto context = GetContext();
			if (context)
				context->UpdateMoDARTDelay(delay);
		}

		////////////////////////////////////////

		void UpdateMoDARTMinimumReverbTime(const Real T60)
		{
			auto context = GetContext();
			if (context)
				context->UpdateMoDARTMinimumReverbTime(T60);
		}

		////////////////////////////////////////

		void UpdateSingleFDNReverbTime(const ReverbFormula model)
		{
			auto context = GetContext();
			if (context)
				context->UpdateSingleFDNReverbTime(model);
		}

		////////////////////////////////////////

		void UpdateSingleFDNReverbTime(const Coefficients<>& T60)
		{
			auto context = GetContext();
			if (context)
				context->UpdateSingleFDNReverbTime(T60);
		}

		////////////////////////////////////////

		void UpdateDiffractionModel(const DiffractionModel model)
		{
			auto context = GetContext();
			if (context)
				context->UpdateDiffractionModel(model);
		}

		////////////////////////////////////////

		bool InitEarlyReverb(const bool enabled, const EarlyReverbData& data, const DiffractionModel model)
		{
			auto context = GetContext();
			if (context)
				return context->InitEarlyReverb(enabled, data, model);
			return false;
		}

		////////////////////////////////////////

		bool InitSingleFDN(const RoomData& roomData, const LateReverbData& data)
		{
			auto context = GetContext();
			if (context)
				return context->InitSingleFDN(roomData, data);
			return false;
		}

		////////////////////////////////////////

		bool InitMoDART(const MoDARTData& data)
		{
			auto context = GetContext();
			if (context)
				return context->InitMoDART(data);
			return false;
		}

		////////////////////////////////////////

		void ResetLateReverb()
		{
			auto context = GetContext();
			if (context)
				context->ResetLateReverb();
		}

		////////////////////////////////////////

		void UpdateListener(const Vec3& position, const Vec4& orientation)
		{
			auto context = GetContext();
			if (context)
				context->UpdateListener(position, orientation);
		}

		////////////////////////////////////////

		int InitSource()
		{
			auto context = GetContext();
			if (context)
				return context->InitSource();
			else
				return -1;
		}

		////////////////////////////////////////

		void UpdateSource(const size_t id, const Vec3& position, const Vec4& orientation)
		{
			auto context = GetContext();
			if (context)
				context->UpdateSource(id, position, orientation);
		}

		////////////////////////////////////////

		void UpdateSourceDirectivity(const size_t id, const SourceDirectivity directivity)
		{
			auto context = GetContext();
			if (context)
				context->UpdateSourceDirectivity(id, directivity);
		}

		////////////////////////////////////////

		void RemoveSource(const size_t id)
		{
			auto context = GetContext();
			if (context)
				context->RemoveSource(id);
		}

		////////////////////////////////////////

		int InitMaterial(const Coefficients<>& material)
		{
			auto context = GetContext();
			if (context)
				return context->InitMaterial(material);
			else
				return -1;
		}

		////////////////////////////////////////

		void UpdateMaterial(const size_t id, const Coefficients<>& material)
		{
			auto context = GetContext();
			if (context)
				context->UpdateMaterial(id, material);
		}

		////////////////////////////////////////

		void RemoveMaterial(const size_t id)
		{
			auto context = GetContext();
			if (context)
				context->RemoveMaterial(id);
		}

		////////////////////////////////////////

		int InitWall(const Vertices& vData, const size_t materialId)
		{
			auto context = GetContext();
			if (context)
				return context->InitWall(vData, materialId);
			else
				return -1;
		}

		////////////////////////////////////////

		void UpdateWall(size_t id, const Vertices& vData)
		{
			auto context = GetContext();
			if (context)
				context->UpdateWall(id, vData);
		}

		////////////////////////////////////////

		void RemoveWall(size_t id)
		{
			auto context = GetContext();
			if (context)
				context->RemoveWall(id);
		}

		////////////////////////////////////////

		void UpdatePlanesAndEdges()
		{
			auto context = GetContext();
			if (context)
				context->UpdatePlanesAndEdges();
		}

		////////////////////////////////////////
		
		void UpdateLateReverbGain(const Real gain)
		{
			auto context = GetContext();
			if (context)
				context->UpdateLateReverbGain(gain);
		}

		////////////////////////////////////////

		void SubmitAudio(size_t id, const Buffer<>& data)
		{
			auto context = GetContext();
			if (context)
				context->SubmitAudio(id, data);
		}

		////////////////////////////////////////

		void GetOutput(Buffer<>& outputBuffer)
		{
			auto context = GetContext();
			if (context)
				context->GetOutput(outputBuffer);
			else
				outputBuffer.Reset();
		}

		////////////////////////////////////////

		void RecordImpulseResponse(const Vec3& position, const Vec4& orientation, Buffer<>& outputBuffer)
		{
			auto context = GetContext();
			if (context)
				context->RecordImpulseResponse(position, orientation, outputBuffer);
		}

		////////////////////////////////////////

		void UpdateImpulseResponseMode(const bool mode)
		{
			auto context = GetContext();
			if (context)
				context->UpdateImpulseResponseMode(mode);
		}
	}
}