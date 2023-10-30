#pragma once

#include "Spatialiser/Interface.h"

// Context singleton ptr
static Spatialiser::Context* context = nullptr;


#pragma region Context
namespace Spatialiser
{
	Context* GetContext()
	{
		return context;
	}

	void Init(const Config* config) // Initialise new context
	{
		if (context) // Delete any existing context
		{
			Debug::Log("Delete Existing Context", Color::Red);
			Exit();
		}
		Debug::Log("Create New Context", Color::Green);
		context = new Context(config);
	}

	void Exit() // Delete existing context
	{
		if (context)
		{
			delete context;
			context = nullptr;
		}
	}

	size_t InitWall(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall)
	{
		auto* context = GetContext();
		if (context)
			return context->InitWall(normal, vData, numVertices, absorption, reverbWall);
		else
			return -1;
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

	void SetFDNParameters(const float& volume, const vec& dimensions)
	{
		auto* context = GetContext();
		if (context)
			context->SetFDNParameters(volume, dimensions);
	}

	bool FilesLoaded()
	{
		auto* context = GetContext();
		if (context)
			return context->FilesLoaded();
		else
			return false;
	}

	void UpdateListener(const vec3& position, const quaternion& orientation)
	{
		auto* context = GetContext();
		if (context)
			context->UpdateListener(position, orientation);
	}

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
		{
			Debug::Log("GetOutput Context Exists", Color::Orange);

			context->GetOutput(bufferPtr);
		}
		else
		{
			Debug::Log("GetOutput Context Does Not Exist", Color::Orange);
			*bufferPtr = nullptr;
		}
	}
}
#pragma endregion