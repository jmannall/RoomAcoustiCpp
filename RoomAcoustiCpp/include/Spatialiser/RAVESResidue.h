/*
* @class Residue
*
* @brief Declaration of RAVES Residue class
*
*/

#ifndef RoomAcoustiCpp_RAVESResidue_h
#define RoomAcoustiCpp_RAVESResidue_h

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
		class RAVESResidue
		{
		public:
			RAVESResidue(Real energy) : currentEnergy(energy), targetEnergy(energy)
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
				UpdateResidue(currentEnergy);
			}

			virtual void UpdateResidue(Real energy) = 0;

			// NOTE: residue is always real, even through it's later multiplied by a complex value
			Real residue;		// Residue for the RAVES algorithm

		private:

			Real currentEnergy;
			std::atomic<Real> targetEnergy;

		protected:
			std::atomic<bool> parametersEqual{ false };
		};

		class RAVESSourceResidue : public RAVESResidue
		{
		public:
			RAVESSourceResidue(int frequencyIndex = 0, Real energy = 0.0) : RAVESResidue(energy), frequencyIndex(frequencyIndex)
			{
				UpdateResidue(energy);
			}

			inline Complex GetOutput(Real input, Real lerpFactor)
			{
				if (!parametersEqual.load(std::memory_order_acquire))
					InterpolateParameters(lerpFactor);
				return input * residue;
			}

			virtual void UpdateResidue(Real energy) override { residue = std::sqrt(energy); }

			int frequencyIndex;
		};

		class RAVESListenerResidue : public RAVESResidue
		{
		public:
			RAVESListenerResidue(Real energy = 0.0) : RAVESResidue(energy)
			{
				UpdateResidue(energy);
			}

			inline Real GetOutput(Complex input, Real lerpFactor)
			{
				if (!parametersEqual.load(std::memory_order_acquire))
					InterpolateParameters(lerpFactor);
				return (input * residue).real();
			}

			virtual void UpdateResidue(Real energy) override { residue = std::sqrt(energy); }
		};
	}
}

#endif // RoomAcoustiCpp_RAVESResidue_h
