/*
* @class Buffer, BufferF
*
* @brief Declaration of Buffer and BufferF classes
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
			BandFilter(const size_t& order, const bool& useLowBands, const int& sampleRate)
			{ InitSections(order, useLowBands, sampleRate); }

			BandFilter(const size_t& order, const bool& useLowBands, const Real& fb, const Real& g, const int& sampleRate);

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
		};

		class ParametricEQ
		{
		public:
			ParametricEQ(const size_t& order, const Coefficients& fc, const int& sampleRate);
			ParametricEQ(Coefficients& gain, const size_t& order, const Coefficients& fc, const int& sampleRate);

			void UpdateParameters(Coefficients& gain);
			Real GetOutput(const Real input);

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
			Coefficients fb;
			Real out;
		};
	}
}
#endif