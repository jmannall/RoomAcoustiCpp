#pragma once

#include "Spatialiser/Context.h"

namespace Spatialiser
{
	void Init(const Config* config);

	void Exit();
	
	size_t InitWall(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall);

	void UpdateWall(size_t id, const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall);

	void RemoveWall(size_t id, const ReverbWall& reverbWall);

	void SetFDNParameters(const float& volume, const vec& dimensions);

	bool FilesLoaded();

	void UpdateListener(const vec3& position, const quaternion& orientation);

	size_t InitSource();

	void UpdateSource(size_t id, const vec3& position, const quaternion& orientation);

	void RemoveSource(size_t id);

	void SubmitAudio(size_t id, const float* data, size_t numFrames);

	void GetOutput(float** bufferPtr);
}