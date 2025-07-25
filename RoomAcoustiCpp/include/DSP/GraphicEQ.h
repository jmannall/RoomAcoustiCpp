/*
* @class GraphicEQ
*
* @brief Declaration of Graphic EQ class
* 
* @remarks Based after Efficient Multi-Band Digital Audio Graphic Equalizer with Accurate Frequency Response Control. Oliver R, Jot J. 2015
* This appears to have typos in the formulae. Instead see Audio EQ Cookbook:
* https://webaudio.github.io/Audio-EQ-Cookbook/Audio-EQ-Cookbook.txt
* https://www.w3.org/TR/audio-eq-cookbook/#intro
* 
*/

#ifndef DSP_GraphicEQ_h
#define DSP_GraphicEQ_h

// Common headers
#include "Common/Types.h"
#include "Common/Complex.h"

// DSP headers
#include "DSP/GraphicEQ_private.h"

namespace RAC
{
	namespace DSP
	{
		extern template class GraphicEQ<Real>;
		extern template class GraphicEQ<ComplexPair>;
		extern template class GraphicEQ<Complex>;
	}
}
#endif