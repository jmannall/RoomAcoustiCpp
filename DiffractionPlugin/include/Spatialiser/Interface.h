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
	bool Init(const Config* config);
	void Exit();

	// Image Source Model
	void UpdateISMConfig(const ISMConfig& config);
	
	// Reverb
	void SetFDNParameters(const float& volume, const vec& dimensions);

	// Listener
	void UpdateListener(const vec3& position, const quaternion& orientation);

	// Source
	int InitSource();
	void UpdateSource(size_t id, const vec3& position, const quaternion& orientation);
	void RemoveSource(size_t id);

	// Wall
	int InitWall(const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall);
	void UpdateWall(size_t id, const vec3& normal, const float* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall);
	void RemoveWall(size_t id, const ReverbWall& reverbWall);

	// Audio
	void SubmitAudio(size_t id, const float* data);
	void GetOutput(float** bufferPtr);
}

#endif