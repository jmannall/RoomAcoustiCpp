/*
* @class GraphicEQ
*
* @brief Declaration of Graphic EQ class
*
*/

#ifndef DSP_GraphicEQ_h
#define DSP_GraphicEQ_h

// Common headers
#include "Common/Types.h"
#include "Common/Coefficients.h"
#include "Common/Matrix.h"
#include "Common/Vec.h"

// DSP headers
#include "DSP/IIRFilter.h"
#include "DSP/Interpolate.h"

namespace UIE
{
	namespace DSP
	{
		class GraphicEQ
		{
		public:
			GraphicEQ(const Coefficients& fc, const Real& Q, const int& sampleRate);
			GraphicEQ(const Coefficients& gain, const Coefficients& fc, const Real& Q, const int& sampleRate);
			~GraphicEQ() {};

			void InitParameters(const Coefficients& g);
			inline void UpdateParameters(const Real& lerpFactor)
			{
				if (currentGain != targetGain)
				{
					Lerp(currentGain, targetGain, lerpFactor);
					UpdateParameters();
				}
			}
			inline void SetGain(const Coefficients& g) { targetGain = g; }
			Real GetOutput(const Real& input);
			void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor);

			inline void ClearBuffers()
			{
				lowShelf.ClearBuffers();
				for (PeakingFilter& filter : peakingFilters)
					filter.ClearBuffers();
				highShelf.ClearBuffers();
			}
		private:
			void UpdateParameters();
			void InitFilters(const Coefficients& fc, const Real& Q, const int& sampleRate);
			void InitMatrix(const Coefficients& fc);

			Real numFilters;
			PeakLowShelf lowShelf;
			PeakHighShelf highShelf;
			std::vector<PeakingFilter> peakingFilters;

			matrix mat;

			Coefficients targetGain;
			Coefficients currentGain;
			rowvec dbGain;
			rowvec inputGain;
			// Coefficients fb;
			Real out;

			bool valid;
		};
	}
}
#endif