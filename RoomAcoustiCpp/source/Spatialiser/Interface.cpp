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
#ifdef USE_UNITY_DEBUG
	using namespace Unity;
#endif
	namespace Spatialiser
	{
		////////////////////////////////////////
		// Context Singleton
		static std::shared_ptr<Context> context = nullptr;

		////////////////////////////////////////

		static std::shared_ptr<Context> GetContext() { return context; }

		////////////////////////////////////////

		bool Init(const std::shared_ptr<Config> config)
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

		void UpdateIEMConfig(const IEMData& data)
		{
			auto context = GetContext();
			if (context)
				context->UpdateIEMConfig(data);
		}

		////////////////////////////////////////

		void UpdateReverbTime(const ReverbFormula model)
		{
			auto context = GetContext();
			if (context)
				context->UpdateReverbTime(model);
		}

		////////////////////////////////////////

		void UpdateReverbTime(const Coefficients<>& T60)
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

		bool InitLateReverb(const Real volume, const Vec& dimensions, const FDNMatrix matrix)
		{
			auto context = GetContext();
			if (context)
				return context->InitLateReverb(volume, dimensions, matrix);
			return false;
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

		int InitSource()
		{
			auto context = GetContext();
			if (context)
				return context->InitSource();
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

		int InitWall(const Vertices& vertices, const Absorption<>& absorption)
		{
			auto context = GetContext();
			if (context)
				return context->InitWall(vertices, absorption);
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

		void UpdateWallAbsorption(size_t id, const Absorption<>& absorption)
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