/*
* 
* \Spatialiser API
*
*/

#ifndef Spatialiser_Interface_h
#define Spatialiser_Interface_h

#include "Spatialiser/Context.h"

namespace Spatialiser
{
	// Load and Destroy
	void Init(const Config* config);
	void Exit();
	bool FilesLoaded();

	// Image Source Model
	void UpdateISMConfig(const ISMConfig& config);
	
	// Reverb
	void SetFDNParameters(const float& volume, const vec& dimensions);

	// Listener
	void UpdateListener(const vec3& position, const quaternion& orientation);

	// Source
	size_t InitSource();
	void UpdateSource(size_t id, const vec3& position, const quaternion& orientation);
	void RemoveSource(size_t id);

	// Wall
	size_t InitWall(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall);
	void UpdateWall(size_t id, const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall);
	void RemoveWall(size_t id, const ReverbWall& reverbWall);

	// Audio
	void SubmitAudio(size_t id, const float* data, size_t numFrames);
	void GetOutput(float** bufferPtr);
}

#endif