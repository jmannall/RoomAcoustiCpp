#pragma once

#include "Definitions.h"
#include "Types.h"

// Include files
#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>
#include "myBestNN.h"
#include "myNN_initialize.h"
#include "myNN_terminate.h"
#include "mySmallNN.h"

namespace GA
{
	// GA_API
	void InitGeometry(const DSPConfig* config);

	void ExitGeometry();

	void SetListenerPosition(const vec3& position);

	void SetModel(const Model& model);

	size_t InitSource(const vec3& position);

	void RemoveSource(size_t id);

	void UpdateSourceData(size_t id, const vec3& position);

	size_t InitWedge(const Wedge& wedge);

	void RemoveWedge(size_t id);

	void UpdateWedgeData(size_t id, const Wedge& wedge);

	float GetZ(size_t sID, size_t wID);

	float GetSd(size_t sID, size_t wID);

	float GetRd(size_t sID, size_t wID);

	void SendAudio(size_t sID, size_t wID, const float* data, unsigned numFrames);

	void GetOutput(float** buffer);

	void UpdatePaths();
}