/*
* @class Directvity
*
* @brief Declaration of Directivity class
*
*/

#ifndef RoomAcoustiCpp_Directivity_h
#define RoomAcoustiCpp_Directivity_h

// Common headers
#include "Common/Types.h"
#include "Common/Definitions.h"
#include "Common/Complex.h"
#include "Common/Coefficients.h"

// C++ headers
#include <vector>

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		/**
		* @brief Class that stores directivity as spherical harmonics for given frequency bands
		*/
		class Directivity
		{
		public:
			/**
			* @brief Constructor that initialises the Directivity
			* 
			* @params Centre frequencies of the spherical harmonics
			* @params Spherical harmonic coefficients
			*/
			Directivity(std::vector<Real> fc, std::vector<std::vector<Complex>> coefficients, std::vector<Real> invDirectivityFactor) : coefficients(coefficients), invDirectivityFactor(invDirectivityFactor)
			{
				for (int i = 0; i < fc.size() - 1; ++i)
					fm.push_back(fc[i] * sqrt(fc[i + 1] / fc[i]));
				
				CalculateOmniResponse();
			}

			/**
			* @brief Calculate the directivity response for a given frequencies and direction
			* 
			* @params frequencies The frequencies to calculate the directivity for
			* @params theta 0 to PI (0 to 180 degrees) 0 points along the forward axis, PI/2 perpendicular to the forward axis and PI is opposing the forward axis
			* @params phi 0 to 2PI (360 degrees) where 0 is the front, top or rear of the source and PI is the bottom of the source (rotates clockwise around the forward axis).
			* 
			* @remark Uses front-pole orientation (RHS)
			*
			* @return The directivity at the given frequency for a given direction
			*/
			inline Absorption Response(const Coefficients& frequencies, Real theta, Real phi) const
			{
				Absorption output(frequencies.Length());
				for (int i = 0; i < frequencies.Length(); ++i)
					output[i] = SingleResponse(frequencies[i], theta, phi);
				return output;
			}

			inline Coefficients AverageResponse(const Coefficients& frequencies) const
			{
				Coefficients output(frequencies.Length());
				for (int i = 0; i < frequencies.Length(); ++i)
					output[i] = AverageResponse(frequencies[i]);
				return output;
			}

		private:

			/**
			* @brief Calculate the directivity response for a given frequency and direction
			*
			* @params f The frequency to calculate the directivity for
			* @params theta 0 to PI (0 to 180 degrees) 0 points along the forward axis, PI/2 perpendicular to the forward axis and PI is opposing the forward axis
			* @params phi 0 to 2PI (360 degrees) where 0 is the front, top or rear of the source and PI is the bottom of the source (rotates clockwise around the forward axis).
			*
			* @remark Uses front-pole orientation
			*
			* @return The directivity at the given frequency for a given direction
			*/
			inline Real SingleResponse(Real f, Real theta, Real phi) const
			{
				int idx = GetFrequencyIndex(f);
				Complex output = coefficients[idx][0];
				int len = std::sqrt(coefficients[idx].size());
				for (int i = 1; i < len; ++i)
				{
					for (int j = -i; j < i + 1; ++j)
					{
						Complex y = SphericalHarmonic(i, j, theta, phi);
						output += coefficients[idx][i * i + i + j] * y;
					}
				}
				return std::abs(output);
			}

			inline Real AverageResponse(Real f) const
			{
				int idx = GetFrequencyIndex(f);
				return invDirectivityFactor[idx];
			}

			inline int GetFrequencyIndex(Real f) const
			{
				return std::upper_bound(fm.begin(), fm.end(), f) - fm.begin();
			}

			/**
			* @brief Precompute the omni-directional response
			*/
			inline void CalculateOmniResponse()
			{
				for (int i = 0; i < coefficients.size(); ++i)
					coefficients[i][0] = coefficients[i][0] * SphericalHarmonic(0, 0, 0.0, 0.0);
			}

			/**
			* @brief Calculate the spherical harmonics for a given direction and indices
			* 
			* @parmas l Shpherical harmonic order
			* @parmas m Spherical harmonic degree
			* @params theta 0 to PI (0 to 180 degrees) 0 points along the forward axis, PI/2 perpendicular to the forward axis and PI is opposing the forward axis
			* @params phi 0 to 2PI (360 degrees) where 0 is the front, top or rear of the source and PI is the bottom of the source (rotates clockwise around the forward axis).
			*
			* @return The spherical harmonic at the given indices and direction
			*/
			inline Complex SphericalHarmonic(int l, int m, Real theta, Real phi) const 
			{
				bool isNegative = m < 0;
				bool isOdd = m % 2 != 0;

				m = std::abs(m);
				// Compute the associated Legendre polynomial sqrt((2l+1)/(4pi)) sqrt((l-m)!/(l+m)!) P_l^m(cos(theta))
				Real P_lm = NormalisedSHLegendrePlm(l, m, std::cos(theta));

				// Compute the exponential term
				Complex E = std::exp(Complex(0, m * phi));

				if (isNegative)
				{
					E = std::conj(E);
					E = isOdd ? -E : E;
				}

				// Compute spherical harmonic
				return P_lm * E;
			}

			std::vector<Real> fm;								// Mid frequencies
			std::vector<std::vector<Complex>> coefficients;		// Spherical harmonics coefficients
			std::vector<Real> invDirectivityFactor;				// 1 / Directivity Factor (DF) -> DF = 10 ^ (Directivity Index / 20)
		};

		/**
		* @brief Genelec 8020c directivity data as spherical harmonics
		* 
		* @remark Calculated from the BRAS Database directivity measurements
		*/
		const static std::vector<std::vector<Complex>> GENELEC_DIRECTIVITY = {
			{ 2.766636223198812 },
			{ 3.510204359731806 },
			{ 3.370669764960976 },
			
			{ 2.64325252062826,
			Complex(-0.0637887899933280, 0.00709043682858875),
			0.475502894948934,
			Complex(0.0637887899933280, 0.00709043682858875),
			Complex(-0.00766656882903998, 0.00502613096609965),
			Complex(0.00552595525197679, 0.00669867042959554),
			0.146385344721295,
			Complex(-0.00552595525197679, 0.00669867042959554),
			Complex(-0.00766656882903998, -0.00502613096609965) },

			{ 1.97128124928243,
			Complex(0.0288955622920283, 0.0101887659330981),
			0.543116368361039,
			Complex(-0.0288955622920283, 0.0101887659330981),
			Complex(-0.0222949902191432, 0.00482095521649736),
			Complex(0.0143165526072852, 0.00317936142931662),
			0.144068751090373,
			Complex(-0.0143165526072852, 0.00317936142931662),
			Complex(-0.0222949902191432, -0.00482095521649736) },
			
			{ 1.37100588209144,
			Complex(0.0146562216497114, 0.00515044599364579),
			0.720386999496995,
			Complex(-0.0146562216497114, 0.00515044599364579),
			Complex(-0.0650615377560616, 0.00397170007680051),
			Complex(0.0367209089864466, 0.00618685999609908),
			0.351953301197736,
			Complex(-0.0367209089864466, 0.00618685999609908),
			Complex(-0.0650615377560616, -0.00397170007680051),
			Complex(-0.0357384354633624, -0.00265237385647239),
			Complex(-0.0532304055163828, 0.00285715119698396),
			Complex(0.0496970488762515, 0.00389936245744877),
			0.121330122148826,
			Complex(-0.0496970488762515, 0.00389936245744877),
			Complex(-0.0532304055163828, -0.00285715119698396),
			Complex(0.0357384354633624, -0.00265237385647239) },

			{ 1.07987502306489,
			Complex(-0.0159120139766602, 0.0352113761857916),
			0.664568633630202,
			Complex(0.0159120139766602, 0.0352113761857916),
			Complex(-0.0339235653998775, 0.00392905938423214),
			Complex(-0.0136105518477092, 0.0159126538918982),
			0.287751426442446,
			Complex(0.0136105518477092, 0.0159126538918982),
			Complex(-0.0339235653998775, -0.00392905938423214),
			Complex(-0.0394958780644998, -0.00110778262618036),
			Complex(-0.0375235021578686, 0.00291360976273746),
			Complex(0.0348035789199694, 0.00885765396282877),
			0.120961170905888,
			Complex(-0.0348035789199694, 0.00885765396282877),
			Complex(-0.0375235021578686, -0.00291360976273746),
			Complex(0.0394958780644998, -0.00110778262618036) },

			{ 1.02095654461750,
			Complex(0.0241799203216629, 0.0362664242624599),
			0.771135355920134,
			Complex(-0.0241799203216629, 0.0362664242624599),
			Complex(-0.0167508380936951, 0.00257611255022035),
			Complex(0.000676587781659708, 0.0274680154532966),
			0.348051354619507,
			Complex(-0.000676587781659708, 0.0274680154532966),
			Complex(-0.0167508380936951, -0.00257611255022035),
			Complex(-0.00414471799601657, -0.00130780159524470),
			Complex(-0.0342853542828136, 0.00308769607090894),
			Complex(0.0152598563883195, 0.0152615542290605),
			0.106841233812882,
			Complex(-0.0152598563883195, 0.0152615542290605),
			Complex(-0.0342853542828136, -0.00308769607090894),
			Complex(0.00414471799601657, -0.00130780159524470), },

			{ 1.07838488548815,
			Complex(-0.0394941970606158, -0.0388894927032113),
			0.998186478509121,
			Complex(0.0394941970606158, -0.0388894927032113),
			Complex(-0.0374103834948442, 0.00765545389631506),
			Complex(-0.0755795876724971, -0.0353076305014837),
			0.507222386717422,
			Complex(0.0755795876724971, -0.0353076305014837),
			Complex(-0.0374103834948442, -0.00765545389631506),
			Complex(0.0411095458969911, 0.0491546186817065),
			Complex(-0.0416257687194416, 0.00144233306080249),
			Complex(-0.0597357730591135, -0.0255126272371865),
			0.141051073362225,
			Complex(0.0597357730591135, -0.0255126272371865),
			Complex(-0.0416257687194416, -0.00144233306080249),
			Complex(-0.0411095458969911, 0.0491546186817065) }
		};

		/**
		* @brief Genelec 8020c directivity index
		*
		* @remark Calculated from the BRAS Database directivity measurements
		* Stored as: 1 / Directivity Factor (DF) -> DF = 10 ^ (Directivity Index / 20) 
		*/
		const static std::vector<Real> GENELEC_DIRECTIVITY_INDEX = {
			0.996779582927969,
			0.997766996961373,
			0.950922228757372,
			0.737818486582158,
			0.613632382845031,
			0.397746067265630,
			0.324358636045665,
			0.319287031537848,
			0.275016364667025
		};

		/**
		* @brief Genelec 8020c directivity data
		*/
		const Directivity GENELEC = Directivity({ 62.5, 125.0, 250.0, 500.0, 1e3, 2e3, 4e3, 8e3, 16e3 }, GENELEC_DIRECTIVITY, GENELEC_DIRECTIVITY_INDEX);
	}
}
#endif