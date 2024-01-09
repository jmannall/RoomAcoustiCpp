/*
*
* \Spatialiser API
*
*/

#include "Spatialiser/Interface.h"

namespace UIE
{
	namespace Spatialiser
	{
		// Context Singleton

		static Context* context = nullptr;

		Context* GetContext() { return context; }

		// Load and Destroy

		bool Init(const Config* config)
		{
			if (context) // Delete any existing context
			{
				Debug::Log("Delete Existing Context", Colour::Red);
				Exit();
			}
			Debug::Log("Create New Context", Colour::Green);
			context = new Context(config);
			return context->IsRunning();
		}

		void Exit()
		{
			if (context)
			{
				delete context;
				context = nullptr;
			}
		}

		// Image Source Model

		void UpdateISMConfig(const ISMConfig& config)
		{
			auto* context = GetContext();
			if (context)
				context->UpdateISMConfig(config);
		}

		// Reverb

		void SetFDNParameters(const Real& volume, const vec& dimensions)
		{
			auto* context = GetContext();
			if (context)
				context->SetFDNParameters(volume, dimensions);
		}

		// Listener

		void UpdateListener(const vec3& position, const vec4& orientation)
		{
			auto* context = GetContext();
			if (context)
				context->UpdateListener(position, orientation);
		}

		// Source

		int InitSource()
		{
			auto* context = GetContext();
			if (context)
				return (int)context->InitSource();
			else
				return -1;
		}

		void UpdateSource(size_t id, const vec3& position, const vec4& orientation)
		{
			auto* context = GetContext();
			if (context)
				context->UpdateSource(id, position, orientation);
		}

		void RemoveSource(size_t id)
		{
			auto* context = GetContext();
			if (context)
				context->RemoveSource(id);
		}

		// Wall

		int InitWall(const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall)
		{
			auto* context = GetContext();
			if (context)
				return (int)context->InitWall(normal, vData, numVertices, absorption, reverbWall);
			else
				return -1;
		}

		void UpdateWall(size_t id, const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall)
		{
			auto* context = GetContext();
			if (context)
				context->UpdateWall(id, normal, vData, numVertices, absorption, reverbWall);
		}

		void RemoveWall(size_t id, const ReverbWall& reverbWall)
		{
			auto* context = GetContext();
			if (context)
				context->RemoveWall(id, reverbWall);
		}

		// Audio

		void SubmitAudio(size_t id, const Real* data)
		{
			auto* context = GetContext();
			if (context)
				context->SubmitAudio(id, data);
		}

		void GetOutput(Real** bufferPtr)
		{
			auto* context = GetContext();
			if (context)
				context->GetOutput(bufferPtr);
			else
				*bufferPtr = nullptr;
		}
	}
}