/*
* @brief Interface between the API and Spatialiser global context
* 
*/

// Spatialiser headers
#include "Spatialiser/Interface.h"
#include "Spatialiser/Context.h"

namespace UIE
{
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

		bool LoadSpatialisationFiles(const int& hrtfResamplingStep, const std::vector<std::string>& filePaths)
		{
			auto context = GetContext();
			if (context)
				return context->LoadSpatialisationFiles(hrtfResamplingStep, filePaths);
			else
				return false;
		}

		////////////////////////////////////////

		void UpdateSpatialisationMode(const SPATConfig& config)
		{
			auto context = GetContext();
			if (context)
				context->UpdateSpatialisationMode(config);
		}

		////////////////////////////////////////

		void UpdateISMConfig(const ISMConfig& config)
		{
			auto context = GetContext();
			if (context)
				context->UpdateISMConfig(config);
		}

		////////////////////////////////////////

		void UpdateReverbTimeModel(const ReverbTime& model)
		{
			auto context = GetContext();
			if (context)
				context->UpdateReverbTimeModel(model);
		}

		////////////////////////////////////////

		void UpdateFDNModel(const FDNMatrix& model)
		{
			auto context = GetContext();
			if (context)
				context->UpdateFDNModel(model);
		}

		////////////////////////////////////////

		void UpdateRoom(const Real& volume, const vec& dimensions)
		{
			auto context = GetContext();
			if (context)
				context->UpdateRoom(volume, dimensions);
		}

		////////////////////////////////////////

		void UpdateListener(const vec3& position, const vec4& orientation)
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
				return (int)context->InitSource();
			else
				return -1;
		}

		////////////////////////////////////////

		void UpdateSource(size_t id, const vec3& position, const vec4& orientation)
		{
			auto context = GetContext();
			if (context)
				context->UpdateSource(id, position, orientation);
		}

		////////////////////////////////////////

		void RemoveSource(size_t id)
		{
			auto context = GetContext();
			if (context)
				context->RemoveSource(id);
		}

		////////////////////////////////////////

		int InitWall(const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption)
		{
			auto context = GetContext();
			if (context)
				return (int)context->InitWall(normal, vData, numVertices, absorption);
			else
				return -1;
		}

		////////////////////////////////////////

		void UpdateWall(size_t id, const vec3& normal, const Real* vData, size_t numVertices)
		{
			auto context = GetContext();
			if (context)
				context->UpdateWall(id, normal, vData, numVertices);
		}

		////////////////////////////////////////

		void FreeWallId(size_t id)
		{
			auto context = GetContext();
			if (context)
				context->FreeWallId(id);
		}

		////////////////////////////////////////

		void RemoveWall(size_t id)
		{
			auto context = GetContext();
			if (context)
				context->RemoveWall(id);
		}

		void UpdatePlanesAndEdges()
		{
			auto context = GetContext();
			if (context)
				context->UpdatePlanesAndEdges();
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
	}
}