/*
*
* \Spatialiser API
*
*/

#include "Spatialiser/Interface.h"

namespace Spatialiser
{
	// Context Singleton

	static Context* context = nullptr;

	Context* GetContext() { return context; }

	// Load and Destroy

	void Init(const Config* config)
	{
		if (context) // Delete any existing context
		{
			Debug::Log("Delete Existing Context", Color::Red);
			Exit();
		}
		Debug::Log("Create New Context", Color::Green);
		context = new Context(config);
	}

	void Exit()
	{
		if (context)
		{
			delete context;
			context = nullptr;
		}
	}

	bool FilesLoaded()
	{
		auto* context = GetContext();
		if (context)
			return context->FilesLoaded();
		else
			return false;
	}

	// Image Source Model

	void UpdateISMConfig(const ISMConfig& config)
	{
		auto* context = GetContext();
		if (context)
			context->UpdateISMConfig(config);
	}

	// Reverb

	void SetFDNParameters(const float& volume, const vec& dimensions)
	{
		auto* context = GetContext();
		if (context)
			context->SetFDNParameters(volume, dimensions);
	}

	// Listener

	void UpdateListener(const vec3& position, const quaternion& orientation)
	{
		auto* context = GetContext();
		if (context)
			context->UpdateListener(position, orientation);
	}

	// Source

	size_t InitSource()
	{
		auto* context = GetContext();
		if (context)
			return context->InitSource();
		else
			return (size_t)(-1);
	}

	void UpdateSource(size_t id, const vec3& position, const quaternion& orientation)
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

	size_t InitWall(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall)
	{
		auto* context = GetContext();
		if (context)
			return context->InitWall(normal, vData, numVertices, absorption, reverbWall);
		else
			return (size_t)(-1);
	}

	void UpdateWall(size_t id, const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall)
	{
		auto* context = GetContext();
		if (context)
			context->UpdateWall(id, normal, vData, numVertices , absorption, reverbWall);
	}

	void RemoveWall(size_t id, const ReverbWall& reverbWall)
	{
		auto* context = GetContext();
		if (context)
			context->RemoveWall(id, reverbWall);
	}

	// Audio

	void SubmitAudio(size_t id, const float* data, size_t numFrames)
	{
		auto* context = GetContext();
		if (context)
			context->SubmitAudio(id, data, numFrames);
	}

	void GetOutput(float** bufferPtr)
	{
		auto* context = GetContext();
		if (context)
			context->GetOutput(bufferPtr);
		else
			*bufferPtr = nullptr;
	}
}
