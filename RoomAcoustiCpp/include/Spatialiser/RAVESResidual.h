/*
* @class Residual
*
* @brief Declaration of RAVES Residual class
*
*/

#ifndef RoomAcoustiCpp_RAVESResidual_h
#define RoomAcoustiCpp_RAVESResidual_h

// Common headers
#include "Common/Types.h"
#include "Common/Complex.h"

// DSP headers
#include "DSP/Interpolate.h"

namespace RAC
{
	using namespace Common;
	using namespace DSP;
	namespace Spatialiser
	{
		class RAVESResidual
		{
		public:
			RAVESResidual(Real energy) : currentEnergy(energy), targetEnergy(energy)
			{
				parametersEqual.store(true, std::memory_order_release);
			}

			inline void SetTargetEnergy(Real energy)
			{
				targetEnergy.store(energy, std::memory_order_release);
				parametersEqual.store(false, std::memory_order_release);
			}

			// source to FDN real (audio) (multiply by complex (sqrt(energy)) becomes complex
			// FDN to listener complex (audio) (multiply by complex (sqrt(energy)) then take real
		
		protected:
			void InterpolateParameters(const Real lerpFactor)
			{
				parametersEqual.store(true, std::memory_order_release); // Prevents issues in case targetFc/Gain updated during this function call
				const Real energy = targetEnergy.load(std::memory_order_acquire);
				currentEnergy = Lerp(currentEnergy, energy, lerpFactor);
				if (Equals(currentEnergy, energy))
					currentEnergy = energy;
				else
					parametersEqual.store(false, std::memory_order_release);
				UpdateResidual(currentEnergy);
			}

			virtual void UpdateResidual(Complex energy) = 0;

			Complex residual;		// Residual for the RAVES algorithm

		private:

			Real currentEnergy;
			std::atomic<Real> targetEnergy;

		protected:
			std::atomic<bool> parametersEqual{ false };
		};

		class RAVESSourceResidual : public RAVESResidual
		{
		public:
			RAVESSourceResidual(Real energy = 0.0) : RAVESResidual(energy)
			{
				UpdateResidual(energy);
			}

			inline Complex GetOutput(Real input, Real lerpFactor)
			{
				if (!parametersEqual.load(std::memory_order_acquire))
					InterpolateParameters(lerpFactor);
				return input * residual;
			}

			inline void UpdateResidual(Complex energy) override { residual = std::conj(std::sqrt(energy)); }
		};

		class RAVESListenerResidual : public RAVESResidual
		{
		public:
			RAVESListenerResidual(Real energy = 0.0) : RAVESResidual(energy)
			{
				UpdateResidual(energy);
			}

			inline Real GetOutput(Complex input, Real lerpFactor)
			{
				if (!parametersEqual.load(std::memory_order_acquire))
					InterpolateParameters(lerpFactor);
				return (input * residual).real();
			}

			inline void UpdateResidual(Complex energy) override { residual = std::sqrt(energy); }
		};
	}
}

#endif // RoomAcoustiCpp_RAVESResidual_h
