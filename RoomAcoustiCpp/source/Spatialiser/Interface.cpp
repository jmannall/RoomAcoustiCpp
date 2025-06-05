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

		bool Init(const Config& config)
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
			context = std::make_shared<Context>(config);
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

		void SetHeadphoneEQ(const Buffer& leftIR, const Buffer& rightIR)
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

		void UpdateIEMConfig(const IEMConfig& config)
		{
			auto context = GetContext();
			if (context)
				context->UpdateIEMConfig(config);
		}

		////////////////////////////////////////

		void UpdateReverbTime(const ReverbFormula model)
		{
			auto context = GetContext();
			if (context)
				context->UpdateReverbTime(model);
		}

		////////////////////////////////////////

		void UpdateReverbTime(const Coefficients& T60)
		{
			auto context = GetContext();
			if (context)
				context->UpdateReverbTime(T60);
		}

		////////////////////////////////////////

		void UpdateDiffractionModel(const DiffractionModel model)
		{
			auto context = GetContext();
			if (context)
				context->UpdateDiffractionModel(model);
		}

		////////////////////////////////////////

		void InitLateReverb(const Real volume, const Vec& dimensions, const FDNMatrix matrix)
		{
			auto context = GetContext();
			if (context)
				context->InitLateReverb(volume, dimensions, matrix);
		}

		////////////////////////////////////////

		void ResetFDN()
		{
			auto context = GetContext();
			if (context)
				context->ResetFDN();
		}

		////////////////////////////////////////

		void UpdateListener(const Vec3& position, const Vec4& orientation)
		{
			auto context = GetContext();
			if (context)
				context->UpdateListener(position, orientation);
		}

		////////////////////////////////////////

		size_t InitSource()
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

		size_t InitWall(const Vertices& vData, const Absorption& absorption)
		{
			auto context = GetContext();
			if (context)
				return context->InitWall(vData, absorption);
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

		void UpdateWallAbsorption(size_t id, const Absorption& absorption)
		{
			auto context = GetContext();
			if (context)
				context->UpdateWallAbsorption(id, absorption);
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

		void SubmitAudio(size_t id, const float* data)
		{
			auto context = GetContext();
			if (context)
				context->SubmitAudio(id, data);
		}

		////////////////////////////////////////

		void GetOutput(float** bufferPtr)
		{
			auto context = GetContext();
			if (context)
				context->GetOutput(bufferPtr);
			else
				*bufferPtr = nullptr;
		}

		////////////////////////////////////////

		void UpdateImpulseResponseMode(const Real lerpFactor, const bool mode)
		{
			auto context = GetContext();
			if (context)
				context->UpdateImpulseResponseMode(lerpFactor, mode);
		}
	}
}