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

// #define HAS_GSL

#ifdef HAS_GSL
// GSL headers
#include <gsl/gsl_sf_legendre.h>

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
			Directivity(std::vector<Real> fc, std::vector<std::vector<Complex>> coefficients) : coefficients(coefficients)
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
			* @remark Uses front-pole orientation
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
				int idx = std::upper_bound(fm.begin(), fm.end(), f) - fm.begin();
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
				return output.real();
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
				// Compute the associated Legendre polynomial sqrt((2l+1)/(4pi)) sqrt((l-m)!/(l+m)!) P_l^m(cos(theta))
				Real P_lm = gsl_sf_legendre_sphPlm(l, std::abs(m), std::cos(theta));

				// Compute the exponential term
				Complex E = std::exp(Complex(0, -m * phi));

				// Compute spherical harmonic
				return P_lm * E;
			}

			std::vector<Real> fm;								// Mid frequencies
			std::vector<std::vector<Complex>> coefficients;		// Spherical harmonics coefficients
		};

		/**
		* @brief Genelec 8020c directivity data as spherical harmonics
		* 
		* @remark Calculated from the BRAS Database directivity measurements
		*/
		const static std::vector<std::vector<Complex>> GENELEC_DIRECTIVITY = {
			{ 2.76663622319881 },
			{ 3.51020435973181 },
			{ 3.37066976496098 },
			{ 2.64325252062826,
			Complex(0.0637887899933281, 0.00709043682858882),
			0.475502894948934,
			Complex(0.0637887899933281, -0.00709043682858882),
			Complex(-0.00766656882904000, -0.00502613096609966),
			Complex(-0.00552595525197680, 0.00669867042959554),
			0.146385344721295,
			Complex(-0.00552595525197680, -0.00669867042959554),
			Complex(-0.00766656882904000, 0.00502613096609966) },
			{ 1.97128124928243,
			Complex(-0.0288955622920283, 0.0101887659330982),
			0.543116368361039,
			Complex(-0.0288955622920283, -0.0101887659330982),
			Complex(-0.0222949902191432, -0.00482095521649733),
			Complex(-0.0143165526072853, 0.00317936142931662),
			0.144068751090373,
			Complex(-0.0143165526072853, -0.00317936142931662),
			Complex(-0.0222949902191432, 0.00482095521649733) },
			{ 1.37100588209144,
			Complex(-0.0146562216497115, 0.00515044599364574),
			0.720386999496995,
			Complex(-0.0146562216497115, -0.00515044599364574),
			Complex(-0.0650615377560617, -0.00397170007680053),
			Complex(-0.0367209089864466, 0.00618685999609909),
			0.351953301197736,
			Complex(-0.0367209089864466, -0.00618685999609909),
			Complex(-0.0650615377560617, 0.00397170007680053),
			Complex(0.0357384354633624, -0.00265237385647243),
			Complex(-0.0532304055163828, -0.00285715119698395),
			Complex(-0.0496970488762515, 0.00389936245744878),
			0.121330122148826,
			Complex(-0.0496970488762515, -0.00389936245744878),
			Complex(-0.0532304055163828, 0.00285715119698395),
			Complex(0.0357384354633624, 0.00265237385647243) },
			{ 1.07987502306489,
			Complex(0.0159120139766602, 0.0352113761857916),
			0.664568633630202,
			Complex(0.0159120139766602, -0.0352113761857916),
			Complex(-0.0339235653998775, -0.00392905938423215),
			Complex(0.0136105518477092, 0.0159126538918981),
			0.287751426442446,
			Complex(0.0136105518477092, -0.0159126538918981),
			Complex(-0.0339235653998775, 0.00392905938423215),
			Complex(0.0394958780644998, -0.00110778262618034),
			Complex(-0.0375235021578686, -0.00291360976273745),
			Complex(-0.0348035789199694, 0.00885765396282876),
			0.120961170905888,
			Complex(-0.0348035789199694, -0.00885765396282876),
			Complex(-0.0375235021578686, 0.00291360976273745),
			Complex(0.0394958780644998, 0.00110778262618034) },
			{ 1.02095654461750,
			Complex(-0.0241799203216629, 0.0362664242624598),
			0.771135355920134,
			Complex(-0.0241799203216629, -0.0362664242624598),
			Complex(-0.0167508380936952, -0.00257611255022033),
			Complex(-0.000676587781659705, 0.0274680154532966),
			0.348051354619507,
			Complex(-0.000676587781659705, -0.0274680154532966),
			Complex(-0.0167508380936952, 0.00257611255022033),
			Complex(0.00414471799601655, -0.00130780159524470),
			Complex(-0.0342853542828136, -0.00308769607090893),
			Complex(-0.0152598563883195, 0.0152615542290605),
			0.106841233812882,
			Complex(-0.0152598563883195, -0.0152615542290605),
			Complex(-0.0342853542828136, 0.00308769607090893),
			Complex(0.00414471799601655, 0.00130780159524470) },
			{ 1.07838488548815,
			Complex(0.0394941970606157, -0.0388894927032113),
			0.998186478509120,
			Complex(0.0394941970606157, 0.0388894927032113),
			Complex(-0.0374103834948442, -0.00765545389631503),
			Complex(0.0755795876724971, -0.0353076305014837),
			0.507222386717422,
			Complex(0.0755795876724971, 0.0353076305014837),
			Complex(-0.0374103834948442, 0.00765545389631503),
			Complex(-0.0411095458969911, 0.0491546186817065),
			Complex(-0.0416257687194416, -0.00144233306080249),
			Complex(0.0597357730591135, -0.0255126272371865),
			0.141051073362225,
			Complex(0.0597357730591135, 0.0255126272371865),
			Complex(-0.0416257687194416, 0.00144233306080249),
			Complex(-0.0411095458969911, -0.0491546186817065) }
		};

		/**
		* @brief Genelec 8020c directivity data
		*/
		const Directivity GENELEC = Directivity({ 62.5, 125.0, 250.0, 500.0, 1e3, 2e3, 4e3, 8e3, 16e3 }, GENELEC_DIRECTIVITY);
	}
}
#endif
#endif