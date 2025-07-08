/*
* @class Parameter
*
* @brief Stores an atomic target and current parameter and handles interpolation between them.
*
*/

#ifndef DSP_Parameter_h
#define DSP_Parameter_h

// C++ headers
#include <atomic>

// Common headers
#include "Common/Types.h"

// DSP headers
#include "DSP/Interpolate.h"

namespace RAC
{
	using namespace Common;
	namespace DSP
	{
		class Parameter
		{
		public:
			Parameter(const Real parameter) : target(parameter), current(parameter) {}

			~Parameter() {}

			inline void SetTarget(const Real parameter)
			{
				if (target.load(std::memory_order_acquire) == parameter)
					return;
				target.store(parameter, std::memory_order_release);
				parametersEqual.store(false, std::memory_order_release);
			}

			inline Real Use(const Real lerpFactor)
			{
				if (!parametersEqual.load(std::memory_order_acquire))
					Interpolate(lerpFactor);
				return current;
			}
			
			// TO DO: Can be incorrect if Interpolate called at the same time
			inline bool IsZero() { return parametersEqual.load(std::memory_order_acquire) && target.load(std::memory_order_acquire) == 0.0; }

			inline void Reset(Real newValue) { current = newValue; parametersEqual.store(false, std::memory_order_release); }

		private:
			inline void Interpolate(const Real lerpFactor)
			{
				parametersEqual.store(true, std::memory_order_release); // Prevents issues in case target updated during this function call
				const Real parameter = target.load(std::memory_order_acquire);
				current = Lerp(current, parameter, lerpFactor);
				if (Equals(current, parameter))
					current = parameter;
				else
					parametersEqual.store(false, std::memory_order_release);
			}

			std::atomic<Real> target;
			Real current;

			std::atomic<bool> parametersEqual{ false };
			std::atomic<bool> isZero{ false };
		};

	}
}
#endif // DSP_Parameter_h