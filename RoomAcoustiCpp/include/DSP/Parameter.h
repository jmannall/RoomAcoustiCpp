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
				if (target.load() == parameter)
					return;
				target.store(parameter);
				parametersEqual.store(false);
			}

			inline Real Use(const Real lerpFactor)
			{
				if (!parametersEqual.load())
					Interpolate(lerpFactor);
				return current;
			}
			
			// TO DO: Can be incorrect if Interpolate called at the same time
			inline bool IsZero() { return parametersEqual.load() && target.load() == 0.0; }

			inline void Reset(Real newValue) { current = newValue; parametersEqual.store(false); }

		private:
			inline void Interpolate(const Real lerpFactor)
			{
				parametersEqual.store(true); // Prevents issues in case target updated during this function call
				const Real parameter = target.load();
				current = Lerp(current, parameter, lerpFactor);
				if (Equals(current, parameter))
					current = parameter;
				else
					parametersEqual.store(false);
			}

			std::atomic<Real> target;
			Real current;

			std::atomic<bool> parametersEqual{ false };
			std::atomic<bool> isZero{ false };
		};

	}
}
#endif // DSP_Parameter_h