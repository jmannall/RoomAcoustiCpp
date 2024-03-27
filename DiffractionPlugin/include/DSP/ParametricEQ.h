/*
* @class ParametricEQ, BandFilter, BandSection
*
* @brief Declaration of ParametricEQ, BandFilter, BandSection classes
*
*/

#ifndef DSP_ParametricEQ_h
#define DSP_ParametricEQ_h

// Common headers
#include "Common/Types.h"
#include "Common/Coefficients.h"

// DSP headers
#include "DSP/IIRFilter.h"

namespace UIE
{
	namespace DSP
	{
		class BandSection : public IIRFilter
		{
		public:
			BandSection(const int& idx, const int& order, const bool& isLowBand, const int& sampleRate);
			BandSection(const Real& fb, const Real& g, const int& m, const int& M, const bool& isLowBand, const int& sampleRate);

			inline void SetUpdatePointer(bool isLowBand)
			{
				if (isLowBand)
					UpdateBand = &BandSection::UpdateLowBand;
				else
					UpdateBand = &BandSection::UpdateHighBand;
			};

			inline void UpdateParameters(Real fb, Real g) { (this->*UpdateBand)(fb, g); };
		private:
			int m;
			int M;

			void UpdateLowBand(const Real& fb, const Real& g);
			void UpdateHighBand(const Real& fb, const Real& g);

			// Function pointer
			void (BandSection::* UpdateBand)(const Real& fb, const Real& g);
		};

		class BandFilter
		{
		public:
			BandFilter(const size_t& order, const bool& useLowBands, const int& sampleRate) : out(0.0)
			{ InitSections(order, useLowBands, sampleRate); }
			BandFilter(const size_t& order, const bool& useLowBands, const Real& fb, const Real& g, const int& sampleRate);
			~BandFilter() {};

			void UpdateParameters(const Real& fb, const Real& g);
			Real GetOutput(const Real& input);

			inline void ClearBuffers()
			{
				for (BandSection& section : sections)
					section.ClearBuffers();
			}

		private:
			void InitSections(const size_t& order, const bool& useLowBands, const int& fs);

			int M;
			std::vector<BandSection> sections;
			Real out;
		};

		class ParametricEQ
		{
		public:
			ParametricEQ(const size_t& order, const Coefficients& fc, const int& sampleRate);
			ParametricEQ(const Coefficients& gain, const size_t& order, const Coefficients& fc, const int& sampleRate);
			~ParametricEQ() {};

			void UpdateParameters();
			void UpdateParameters(const Real& lerpFactor);
			void SetTargetGain(Coefficients& gain);
			Real GetOutput(const Real& input);
			void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const int numFrames, const Real lerpFactor);

			inline void ClearBuffers()
			{
				for (BandFilter& filter : filters)
					filter.ClearBuffers();
			}
		private:
			void InitBands(const size_t& order, const Coefficients& fc, int fs);

			size_t numFilters;
			std::vector<BandFilter> filters;
			Real mGain;
			bool singleGain;
			Coefficients targetGain;
			Coefficients currentGain;
			Coefficients fb;
			Real out;
		};
	}
}
#endif