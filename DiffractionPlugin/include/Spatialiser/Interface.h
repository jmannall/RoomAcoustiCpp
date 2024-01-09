/*
* 
* \Spatialiser API
*
*/

#ifndef Spatialiser_Interface_h
#define Spatialiser_Interface_h

#include "Spatialiser/Context.h"

namespace UIE
{
	using namespace Common;
	namespace Spatialiser
	{
		// Load and Destroy
		bool Init(const Config* config);
		void Exit();

		// Image Source Model
		void UpdateISMConfig(const ISMConfig& config);

		// Reverb
		void SetFDNParameters(const Real& volume, const vec& dimensions);

		// Listener
		void UpdateListener(const vec3& position, const vec4& orientation);

		// Source
		int InitSource();
		void UpdateSource(size_t id, const vec3& position, const vec4& orientation);
		void RemoveSource(size_t id);

		// Wall
		int InitWall(const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall);
		void UpdateWall(size_t id, const vec3& normal, const Real* vData, size_t numVertices, Absorption& absorption, const ReverbWall& reverbWall);
		void RemoveWall(size_t id, const ReverbWall& reverbWall);

		// Audio
		void SubmitAudio(size_t id, const Real* data);
		void GetOutput(Real** bufferPtr);
	}
}
#endif