/*
* @class Model, Attenuate, LPF, UDFABase, UDFA, UDFAI, NN, NNBest, NNSmall, UTD, BTM
*
* @brief Diffraction model classes with base class Model
*
*/

#ifndef RoomAcoustiCpp_Diffraction_Models_h
#define RoomAcoustiCpp_Diffraction_Models_h

// C++ headers
#include <mutex>

// NN headers
#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>
#include "myBestNN.h"
#include "myNN_initialize.h"
#include "myNN_terminate.h"
#include "mySmallNN.h"

// Common headers
#include "Common/Types.h"
#include "Common/Complex.h"

// Spatialiser headers
#include "Spatialiser/Diffraction/Path.h"

// DSP headers
#include "DSP/FIRFilter.h"
#include "DSP/IIRFilter.h"
#include "DSP/LinkwitzRileyFilter.h"
#include "DSP/Parameter.h"

namespace RAC
{
	using namespace Common;
	using namespace DSP;
	namespace Spatialiser
	{
		namespace Diffraction
		{
			/**
			* @brief Base class for all diffraction models
			*/
			class Model
			{
			public:
				/**
				* @brief Default constructor
				*/
				Model() {};

				/**
				* @brief Defualt virtual deconstructor
				*/
				virtual ~Model() {};

				/**
				* @breif Pure virtual function to set target parameters based on the given path. Must be overloaded in the derived classes
				* 
				* @param path The path to set the target parameters from
				*/
				virtual void SetTargetParameters(const Path& path) = 0;

				/**
				* @brief Pure virtual function to process audio buffers. Must be overloaded in the derived classes
				* 
				* @param inBuffer The input audio buffer
				* @param outBuffer The output audio buffer to write to
				* @param lerpFactor The lerp factor for interpolation
				*/
				virtual void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) = 0;

			protected:
				std::atomic<bool> isInitialised{ false };	// True if the model has been initialised, false otherwise
			};

			//////////////////// Attenuate class ////////////////////
			
			/**
			* @brief Class that applies an on/off attenuation based on whether the receiver is in the shadow zone
			*/
			class Attenuate : public Model
			{
			public:
				/**
				* @brief Constructor that initialises the Attenuate model with a given path
				* 
				* @param path The path to set the target parameters from
				*/
				Attenuate(const Path& path) : Model(), gain(CalculateGain(path))
				{
					isInitialised.store(true);
				};

				/**
				* @brief Default deconstructor
				*/
				~Attenuate() {};

				/**
				* @brief Set the target gain based on the given path
				* 
				* @param path The path to set the target parameters from
				*/
				inline void SetTargetParameters(const Path& path) override { gain.SetTarget(CalculateGain(path)); }

				/**
				* @brief Processes the input audio buffer and applies the attenuation
				* 
				* @param inBuffer The input audio buffer
				* @param outBuffer The output audio buffer to write to
				* @param lerpFactor The lerp factor for interpolation
				*/
				void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) override;
			private:

				/**
				* @brief Calculates the gain based on the path
				* 
				* @param path The path to calculate the gain from
				* @return 1.0 if the path is valid and the receiver is in the shadow zone, otherwise 0.0
				*/
				inline Real CalculateGain(const Path& path) const { return (path.valid && path.inShadow) ? 1.0 : 0.0; }
			
				Parameter gain;		// Gain parameter
			};

			//////////////////// LPF class ////////////////////

			/**
			* @brief Class that applies a low-pass filter when the receiver is in the shadow zone
			*/
			class LPF : public Model
			{
			public:
				/**
				* @brief Constructor that initialises the LPF model with a given path and sample rate
				*/
				LPF(const Path& path, const int fs) : Model(), gain(CalculateGain(path)), filter(1000.0, fs)
				{
					isInitialised.store(true);
				}

				/**
				* @brief Default deconstructor
				*/
				~LPF() {};

				/**
				* @brief Set the target gain based on the given path
				* 
				* @param path The path to set the target parameters from
				*/
				inline void SetTargetParameters(const Path& path) override { gain.SetTarget(CalculateGain(path)); }

				/**
				* @brief Processes the input audio buffer and applies the low-pass filter
				* 
				* @param inBuffer The input audio buffer
				* @param outBuffer The output audio buffer to write to
				* @param lerpFactor The lerp factor for interpolation
				*/
				void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) override;
			private:

				/**
				* @brief Calculates the gain based on the path
				*
				* @param path The path to calculate the gain from
				* @return 1.0 if the path is valid and the receiver is in the shadow zone, otherwise 0.0
				*/
				inline Real CalculateGain(const Path& path) const { return (path.valid && path.inShadow) ? 1.0 : 0.0; }

				LowPass1 filter;	// Low-pass filter with cutoff frequency of 1000Hz
				Parameter gain;		// Gain parameter
			};

			//////////////////// UDFA class ////////////////////

			/**
			* @brief Currently implemented UDFAModels
			*/
			enum UDFAModel
			{
				Pierce,
				SingleTerm
			};

			/**
			* @brief Base class for UDFAModels. Currently assumes the apex point lies on the edge
			* 
			* @remark Based after A Universal Filter Approximation of Edge Diffraction for Geometrical Acoustics. Kirsch C and Ewert S 2023
			*/
			template <UDFAModel model>
			class UDFABase : public Model
			{
				static constexpr int numShelvingFilters{ 4 };	// Number of shelving filters used to approximate each UDFA filter
				static constexpr int numUDFAFilters = model == UDFAModel::SingleTerm ? 2 : 4;	// Number of UDFA filters used in the model
				
				using ParametersI = Coefficients<std::array<Real, numShelvingFilters>>;			// Parameters type for the shelving filters
				using ParametersT = Coefficients<std::array<Real, numShelvingFilters + 1>>;		// Parameters type for the target parameters

				/**
				* @brief Struct that stores the target shelving filter parameters for each UDFA filter
				*/
				struct FilterParameters
				{
					ParametersI fc;		// Cut-off frequencies
					ParametersI g;		// Shelving gains
					Real gain;			// Gain

					/**
					* @brief Default constructor with a flat frequency response and zero gain.
					*/
					FilterParameters() : gain(0.0), fc({ 45.0, 350.0, 2800.0, 21700.0 }), g({ 1.0, 1.0, 1.0, 1.0 }) {}
				};

				/**
				* @brief Struct that calculates the UDFA filter parameters for a finite edge
				*/
				struct Parameters
				{
					Real fc;				// Cut-off frequency
					Real gain;				// Gain
					Real blend{ 1.44 };		// Blend factor for the filter
					Real Q{ 0.2 };			// Q factor for the filter

					/**
					* @brief Constructor that initialises the parameters for a given cut-off frequency, gain and time difference
					* 
					* @param f Cut-off frequency for an infinite edge
					* @param g Gain for an infinite edge
					* @param tDiff Time difference between the apex path and path via the top or base of the edge
					*/
					Parameters(Real f, Real g, Real tDiff) : fc(f), gain(g)
					{
						Real halfGain = CalculateHalfGain(fc, tDiff); // eq. 10
						Real halfGainSq = halfGain * halfGain;
						fc *= (1.0 / halfGainSq); // eq. 11
						gain *= halfGain;
						blend = 1.0 + (blend - 1.0) * halfGainSq; // eq. 12a
						Q = 0.5 + (Q - 0.5) * halfGainSq; // eq. 12b
					}

					/**
					* @brief Calculate the DC gain for a half wedge (eq. 10)
					* 
					* @param fc Cut-off frequency for an infinite edge
					* @param tDiff Time delay between the apex path and path via the top or base of the edge
					* @return The DC gain for a half wedge
					*/
					inline Real CalculateHalfGain(Real fc, Real tDiff) const {
						return (2 / PI_1) * atan(PI_1 * sqrt(2.0 * fc * tDiff)); }
				};

				/**
				* @brief Struct that calculates the UDFA filter parameters for an infinite edge
				*/
				struct Constants
				{
					Real tDiffBase{ 0.0 }, tDiffTop{ 0.0 };		// Time difference between the apex path and path via the top or base of the edge
					Real fc1{ 0.0 }, fc2{ 0.0 };				// Cut-off frequencies
					Real gain1{ 0.0 }, gain2{ 0.0 };			// Gains

					/**
					* @brief Constructor that calculates the UDFA filter parameters for a given path
					* 
					* @param path The path to calculate the parameters from
					*/
					Constants(const Path& path)
					{
						Real v = PI_1 / path.wData.t;
						Real cosVpi = cos(v * PI_1);
						Real sinVpi = sin(v * PI_1);

						Real cosVt1 = cos(v * (path.rData.t - path.sData.t));
						Real cosVt2 = cos(v * (path.rData.t + path.sData.t));

						Real dStar = 2.0 * path.sData.r * path.rData.r / (path.sData.d + path.rData.d);
						Real front = 2.0 * SPEED_OF_SOUND / (PI_SQ * dStar); // eq. 4

						Real nV1 = CalcNv(cosVt1, v, cosVpi); // eq. 5
						Real nV2 = CalcNv(cosVt2, v, cosVpi);

						fc1 = front * nV1 * nV1; // eq. 4
						fc2 = front * nV2 * nV2;

						gain1 = CalcGv(cosVt1, cosVpi, sinVpi); // eq. 3
						gain2 = CalcGv(cosVt2, cosVpi, sinVpi);

						if constexpr (model == UDFAModel::SingleTerm)
						{
							// Factor of 0.5 already accounted for in CalcGv
							Real fcTerm = gain1 * sqrt(fc1) + gain2 * sqrt(fc2);
							fc1 = fcTerm * fcTerm; // eq. 7
							gain1 = 1.0;
						}
						else
						{
							if (!path.inShadow) // eq. 2
								gain1 = -gain1;
							if (path.inRelfZone)
								gain2 = -gain2;
						}

						Real d = path.sData.d + path.rData.d;
						tDiffBase = (path.GetD(0.0) - d) / SPEED_OF_SOUND;	// Sec III. A
						tDiffTop = (path.GetD(path.wData.z) - d) / SPEED_OF_SOUND;
					}

					/**
					* @brief Calculate the factor Nv (eq. 5)
					*
					* @param cosVt cos(v*theta) where theta = theta_r plus/minus theta_s
					* @param v The exterior wedge index
					* @param cosVpi cos(v*pi) where v is the exterior wedge index
					*/
					inline Real CalcNv(Real cosVt, Real v, Real cosVpi) const {
						return (v * sqrt(1 - cosVpi * cosVt)) / (cosVpi - cosVt);
					}

					/**
					* @brief Calculate the gain for an infinite wedge (eq. 3)
					*
					* @param cosVt cos(v*theta) where theta = theta_r plus/minus theta_s
					* @param cosVpi cos(v*pi) where v is the exterior wedge index
					* @param sinVpi sin(v*pi) where v is the exterior wedge index
					*/
					inline Real CalcGv(Real cosVt, Real cosVpi, Real sinVpi) const {
						return 0.5 * sinVpi / sqrt(1 - cosVpi * cosVt);
					}
				};

			public:
				/**
				* @brief Constructor that initialises shelving filters based on the given path and sample rate
				* 
				* @params path The path to set the target parameters from
				* @params fs The sample rate for calculating filter coefficients
				*/
				UDFABase(const Path& path, const int fs) : ft(CalcFT(fs)), fi(CalcFI())
				{
					if (!path.valid || (model == UDFAModel::SingleTerm && !path.inShadow))
					{
						for (int j= 0; j < numUDFAFilters; j++)
							InitFilters(FilterParameters(), j, fs);
					}
					else
					{
						std::array<Parameters, numUDFAFilters> parameters = CalculateUDFAParameters(path);
						for (int j = 0; j < numUDFAFilters; j++)
						{
							FilterParameters fp = CalculateParameters(parameters[j]);
							InitFilters(fp, j, fs);
						}
					}
					isInitialised.store(true);
				}

				/**
				* @brief Default deconstructor
				*/
				~UDFABase() {}

				/**
				* @brief Set the target parameters based on the given path
				* 
				* @param path The path to set the target parameters from
				*/
				void SetTargetParameters(const Path& path) override
				{
					if (!path.valid || (model == UDFAModel::SingleTerm && !path.inShadow))
					{
						for (int i = 0; i < numUDFAFilters; i++)
							gain[i]->SetTarget(0.0);
						return;
					}

					std::array<Parameters, numUDFAFilters> parameters = CalculateUDFAParameters(path);
					for (int j = 0; j < numUDFAFilters; j++)
					{
						FilterParameters filterParameters = CalculateParameters(parameters[j]);
						gain[j]->SetTarget(filterParameters.gain);
						for (int i = 0; i < numShelvingFilters; i++)
							filters[j * numShelvingFilters + i]->SetTargetParameters(filterParameters.fc[i], filterParameters.g[i]);
					}
				}

				/**
				* @brief Processes the input audio buffer and applies the UDFA filters
				* 
				* @param inBuffer The input audio buffer
				* @param outBuffer The output audio buffer to write to
				* @param lerpFactor The lerp factor for interpolation
				*/
				void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) override
				{
					if (!isInitialised.load())
						return;

					FlushDenormals();
					for (int i = 0; i < inBuffer.Length(); i++)
					{
						Real outSample = 0.0;
						for (int j = 0; j < numUDFAFilters; j++)
						{
							Real out = filters[j * numShelvingFilters]->GetOutput(inBuffer[i], lerpFactor);
							for (int k = 1; k < numShelvingFilters; k++)
								out = filters[j * numShelvingFilters + k]->GetOutput(out, lerpFactor);
							outSample += out * gain[j]->Use(lerpFactor);
						}
						outBuffer[i] = outSample;
					}
					NoFlushDenormals();
				}

			private:

				/**
				* @brief Initialises the shelving filters approximating a single UDFA filter
				* 
				* @param fp The target shelving filter parameters
				* @param j The index of the UDFA filter
				*/
				inline void InitFilters(const FilterParameters& fp, const int j, const int fs)
				{
					gain[j].emplace(fp.gain);
					for (int i = 0; i < numShelvingFilters; i++)
						filters[j * numShelvingFilters + i].emplace(fp.fc[i], fp.g[i], fs);
				}

				/**
				* @brief Calculates the UDFA filter parameters for a finite edge based on the given path
				* 
				* @param path The path to calculate the parameters from
				* @return An array of Parameters for each UDFA filter
				*/
				std::array<Parameters, numUDFAFilters> CalculateUDFAParameters(const Path& path) const
				{
					Constants constants(path);
					if constexpr (model == UDFAModel::Pierce)
					{
						return std::array<Parameters, numUDFAFilters>({
							Parameters(constants.fc1, constants.gain1, constants.tDiffBase),
							Parameters(constants.fc1, constants.gain1, constants.tDiffTop),
							Parameters(constants.fc2, constants.gain2, constants.tDiffBase),
							Parameters(constants.fc2, constants.gain2, constants.tDiffTop)
							});
					}
					else if (model == UDFAModel::SingleTerm)
					{
						return std::array<Parameters, numUDFAFilters>({
							Parameters(constants.fc1, constants.gain1, constants.tDiffBase),
							Parameters(constants.fc1, constants.gain1, constants.tDiffTop)
							});
					}
					else
						assert(false && "Invalid UDFAModel specified.");
				}

				/**
				* @brief Calculates the target shelving filter parameters for each UDFA filter
				* 
				* @param parameters The parameters of the UDFA filter
				* @return FilterParameters containing the target shelving filter parameters
				*/
				FilterParameters CalculateParameters(const Parameters& parameters) const
				{
					ParametersT gt = CalcGT(parameters);
					ParametersI gd = CalcGD(gt, parameters);

					FilterParameters filterParameters;
					for (int i = 0; i < numShelvingFilters; i++)
						filterParameters.g[i] = gt[i + 1] / gt[i];

					const ParametersI gdSq = gd * gd;
					const ParametersI gSq = filterParameters.g * filterParameters.g;
					filterParameters.fc = fi * ((gdSq - gSq) / (filterParameters.g * (1.0 - gdSq))).Sqrt() * (1.0 + gSq / 12.0); // eq. 20
					filterParameters.gain = gt[0] * parameters.gain * 0.5;
					return filterParameters;
				}

				/**
				* @brief Calculates logarimically space frequencies based on the number of shelving filters (sec IV)
				* 
				* @param fs The sample rate to determine the maximum target frequency
				* @return A ParametersT containing the target frequencies
				*/
				inline ParametersT CalcFT(int fs) const
				{
					ParametersT f(numShelvingFilters + 1);
					Real fMin = log10(10.0);
					Real fMax = log10(static_cast<Real>(fs));

					Real delta = (fMax - fMin) / numShelvingFilters;

					for (int i = 0; i <= numShelvingFilters; i++)
						f[i] = pow(10.0, fMin + delta * i);
					return f;
				}

				/**
				* @brief Calculate the geometric centre between two consecutive target frequencies (sec IV)
				* 
				* @return A ParametersI containing the intermediate frequencies
				*/
				inline ParametersI CalcFI() const
				{
					ParametersI f(numShelvingFilters);
					for (int i = 0; i < numShelvingFilters; i++)
						f[i] = ft[i] * sqrt(ft[i + 1] / ft[i]);
					return f;
				}

				/**
				* @brief Calculates the gain of a given UDFA filter at the target frequencies
				* 
				* @param parameters The UDFA filter parameters
				* @return A ParametersT containing the target gains
				*/
				inline ParametersT CalcGT(const Parameters& parameters) const
				{
					ParametersT gt = ParametersT(numShelvingFilters + 1);
					for (int i = 0; i < numShelvingFilters + 1; i++)
						gt[i] = CalcG(ft[i], parameters);
					return gt;
				}

				/**
				* @brief Calculate intermediate gain step where gd is the ratio between the gains at the intermediate frequency and the previous target frequency (sec IV)
				*
				* @param gt The gains at the target frequencies
				* @param parameters The UDFA filter parameters
				* @return A ParametersI containing the intermediate gains
				*/
				inline ParametersI CalcGD(const ParametersT& gt, const Parameters& parameters) const
				{
					ParametersI gd = ParametersI(numShelvingFilters);
					for (int i = 0; i < numShelvingFilters; i++)
						gd[i] = CalcG(fi[i], parameters) / gt[i];
					return gd;
				}

				/**
				* @brief Calculates the gain at a given frequency using the UDFA filter parameters
				* 
				* @param f The frequency to calculate the gain for
				* @param parameters The UDFA filter parameters
				* @return The gain at the given frequency
				*/
				inline Real CalcG(Real f, const Parameters& parameters) const { return abs(CalcUDFA(f, parameters)); }

				/**
				* @brief Calculates the filter response at a given frequency using the UDFA filter parameters
				*
				* @param f The frequency to calculate the gain for
				* @param parameters The UDFA filter parameters
				* @return The complex valued response at the given frequency
				*/
				inline Complex CalcUDFA(Real f, const Parameters& parameters) const
				{
					Real alpha = 0.5;
					Real r = 1.6;
					return pow(pow(imUnit * f / parameters.fc, 2.0 / parameters.blend) + pow(imUnit * f / (parameters.Q * parameters.fc), 1.0 / (pow(parameters.blend, r))) + Complex(1.0, 0.0), -alpha * parameters.blend / 2.0);
				}

			private:
				const ParametersT ft;	// Target frequencies
				const ParametersI fi;	// Intermediate frequencies (geometric centre of ft)

				std::array<std::optional<Parameter>, numUDFAFilters> gain;									// Gain for each UDFA filter approximation
				std::array<std::optional<HighShelfMatched>, numShelvingFilters* numUDFAFilters> filters;	// Shelving filters approximating each the UDFA filters
			};

			/**
			* @brief Class that implements the two-term pierce solution UDFA model
			*/
			class UDFA : public UDFABase<UDFAModel::Pierce>
			{
			public:
				/**
				* @brief Constructor that intialises the UDFABase base class
				*
				* @params path The path to set the target parameters from
				* @params fs The sample rate for calculating filter coefficients
				*/
				UDFA(const Path& path, int fs) : UDFABase(path, fs) {}
			};

			/**
			* @brief Class that implements the single-term solution UDFA model
			*/
			class UDFAI : public UDFABase<UDFAModel::SingleTerm>
			{
			public:
				/**
				* @brief Constructor that intialises the UDFABase base class
				*
				* @params path The path to set the target parameters from
				* @params fs The sample rate for calculating filter coefficients
				*/
				UDFAI(const Path& path, int fs) : UDFABase(path, fs) {}
			};

			//////////////////// NN class ////////////////////

			/**
			* @brief Class that implements a neural network based diffraction model
			* 
			* @remark Based after Efficient diffraction modeling using neural networks and infinite impulse response filters. Mannall J et al. 2023
			* Current models only accurate at 48kHz sample rate
			*/
			class NN : public Model
			{	
				static constexpr int numInputs = 8;			// Number of inputs to the neural network
				
				using Input = std::array<float, numInputs>;		// Input type for the neural network

				/**
				* @brief Struct that stores the output parameters from the neural network
				*/
				struct Parameters
				{
				public:
					Coefficients<std::array<Real, 5>> data{ 0.0 };	// Output parameters (z1, z2, p1, p2, k)

					Parameters(float z[2], float p[2], float k)
					{
						if (z[0] < z[1])
						{
							data[0] = static_cast<Real>(z[0]);
							data[1] = static_cast<Real>(z[1]);
						}
						else
						{
							data[0] = static_cast<Real>(z[1]);
							data[1] = static_cast<Real>(z[0]);
						}

						if (p[0] < p[1])
						{
							data[2] = static_cast<Real>(p[0]);
							data[3] = static_cast<Real>(p[1]);
						}
						else
						{
							data[2] = static_cast<Real>(p[1]);
							data[3] = static_cast<Real>(p[0]);
						}
						data[4] = k;
					}
				};

			public:
				/**
				* @brief Constructor that initialises the neural network with a given path
				* 
				* @param path The path to set the target parameters from
				* @param nnFunction The function pointer to the neural network function
				*/
				NN(const Path& path, void(*nnFunction)(const float*, float*, float*, float*)) : nnFunction(nnFunction), Model(), filter(CalculateParameters(path).data, 48000)
				{
					if (!path.valid)
						filter.SetTargetGain(0.0);
					isInitialised.store(true);
				};

				/**
				* @brief Default virtual deconstructor
				*/
				virtual ~NN() {};

				/**
				* @brief Set the target parameters based on the given path
				* 
				* @param path The path to set the target parameters from
				*/
				void SetTargetParameters(const Path& path) override;

				/**
				* @brief Processes the input audio buffer and applies the ZPK filter
				* 
				* @param inBuffer The input audio buffer
				* @param outBuffer The output audio buffer to write to
				* @param lerpFactor The lerp factor for interpolation
				*/
				void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) override;

			private:
				/**
				* @brief Run the neural network with the given input
				* 
				* @param input The input to the neural network
				* @return The output parameters from the neural network
				*/
				inline Parameters RunNN(const Input& input) const
				{
					float z[2], p[2], k;
					nnFunction(input.data(), z, p, &k);
					return Parameters(z, p, k);
				}

				/**
				* @brief Calculate the target filter parameters based on the given path
				* 
				* @param path The path to calculate the target parameters from
				* @return Parameters containing the target filter parameters
				*/
				Parameters CalculateParameters(const Path& path) const;

				/**
				* @brief Calculate the input to the neural network based on the given path
				* 
				* @param path The path to calculate the input from
				* @return Input containing the neural network input parameters
				*/
				Input CalculateInput(const Path& path) const;

				/**
				* @brief Assign the r and z values based on such that z1 is less than zW / 2. Ensures reciprocity
				* 
				* @param one The SRData with shortest r value
				* @param two The SRData with longest r value
				* @param zW The length of the edge
				* @param input The input to assign the r and z values to
				*/
				void AssignInputRZ(const SRData& one, const SRData& two, Real zW, Input& input) const;

				std::function<void(const float*, float*, float*, float*)> nnFunction;	// Function pointer to the neural network function
				ZPKFilter filter;														// ZPK filter
			};

			/**
			* @brief Class that implements the best neural network model (~10kFLOPS)
			*/
			class NNBest : public NN
			{
			public:
				/**
				* @brief Constructor that intialises the NN base class
				*
				* @params path The path to set the target parameters from
				*/
				NNBest(const Path& path) : NN(path, &myBestNN) {};

				/**
				* @brief Default deconstructor
				*/
				~NNBest() {};
			};

			/**
			* @brief Class that implements the small neural network model (~2kFLOPS)
			*/
			class NNSmall : public NN
			{
			public:
				/**
				* @brief Constructor that intialises the NN base class
				*
				* @params path The path to set the target parameters from
				*/
				NNSmall(const Path& path) : NN(path, &mySmallNN) {};

				/**
				* @brief Default deconstructor
				*/
				~NNSmall() {};
			};

			//////////////////// UTD class ////////////////////

			/**
			* @brief Class that implements the Uniform Theory of Diffraction (UTD) model using a Linkwitz Riley filterbank
			* 
			* @remark Based after A uniform geometrical theory of diffraction for an edge in a perfectly conducting surface. Kouyoumjian R and Pathak P 1974 10.1109/PROC.1974.9651,
			* Sound diffraction by a many-sided barrier or pillar. Kawai T 1981 10.1016/0022-460X(81)90370-9,
			* High-order diffraction and diffuse reflections for interactive sound propagation in large environments. Schissler et al. 2014,
			* Fast Diffraction Pathfinding for Dynamic Sound Propagation. Schissler et al. 2021
			*/
			class UTD : public Model
			{
				using Parameters = Coefficients<std::array<Real, 4>>;	// Parameters type that stores 4 values

				/**
				* @brief Initialises the constants E used in the UTD calculations
				*/
				constexpr std::array<Complex, 4> InitE()
				{
					std::array<Complex, 4> E;
					for (int i = 0; i < 4; ++i)
						E[i] = std::exp(-imUnit * (PI_1 / 4.0)) / (2.0 * std::sqrt(PI_2 * k[i])); // eq. 25
					return E;
				}

			public:
				/**
				* @brief Constructor that initialises the UTD model with a given path and sample rate
				* 
				* @param path The path to set the target parameters from
				* @param fs The sample rate for calculating filter coefficients
				*/
				UTD(const Path& path, int fs) : Model(), lrFilter(CalculateUTD(path), fs)
				{
					isInitialised.store(true);
				}

				/**
				* @brief Default deconstructor
				*/
				~UTD() {};

				/**
				* @brief Set the target parameters based on the given path
				* 
				* @param path The path to set the target parameters from
				*/
				inline void SetTargetParameters(const Path& path) override { lrFilter.SetTargetGains(CalculateUTD(path)); }

				/**
				* @brief Processes the input audio buffer and applies the LinkwitzRiley filter
				* 
				* @param inBuffer The input audio buffer
				* @param outBuffer The output audio buffer to write to
				* @param lerpFactor The lerp factor for interpolation
				*/
				void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) override;

			private:
				/**
				* @brief Calculates the UTD gain parameters for a given path
				* 
				* @param path The path to calculate the parameters from
				*/
				Parameters CalculateUTD(const Path& path) const;

				/**
				* @brief Calculate the coefficient parts for either thetaR plus/minus thetaS
				* 
				* @param t Either thetaR + thetaS or thetaR - thetaS
				* @param k The wave number
				* @param n The exterior wedge index
				* @param L Distance parameter
				* @return Complex value coefficient
				*/
				inline Complex EqHalf(Real t, Real k, Real n, Real L) const { return EqQuarter(t, true, k, n, L) + EqQuarter(t, false, k, n, L); }
				
				/**
				* @brief Calculate the coefficient parts for either thetaR plus/minus thetaS
				*
				* @param t Either thetaR + thetaS or thetaR - thetaS
				* @param plus True if calculating pi + t, false if calculating pi - t
				* @param k The wave number
				* @param n The exterior wedge index
				* @param L Distance parameter
				* @return Complex value coefficient
				*/
				Complex EqQuarter(Real t, bool plus, Real k, Real n, Real L) const;

				/**
				* @brief Flip the sign of t if required
				* 
				* @param t The theta value to flip
				* @param plus True if calculating pi + t, false if calculating pi - t
				* @return t if plus is true, -t if plus is false
				*/
				inline Real PM(Real t, bool plus) const { if (plus) return t; else return -t; }

				

				/**
				* @brief Calculate the alpha value for either thetaR plus/minus thetaS
				* 
				* @param t Either thetaR + thetaS or thetaR - thetaS
				* @param plus True if calculating pi + t, false if calculating pi - t
				* @param n The exterior wedge index
				* @return The alpha value for the given parameters
				*/
				Real AlphaPM(Real t, bool plus, Real n) const;

				/**
				* @brief Calculate the cosine input for AlphaPM from t
				*
				* @param t Either thetaR + thetaS or thetaR - thetaS
				* @param plus True if calculating pi + t, false if calculating pi - t
				* @param n The exterior wedge index
				* @return The cosine input for AlphaPM
				*/
				Real CalculateAlphaPMCosineInput(Real t, bool plus, Real n) const;

				/**
				* @brief Calculate the Fresnel integral for a given x value
				* 
				* @param x The x value to calculate the Fresnel integral for
				* @return The complex valued Fresnel integral value
				*/
				Complex FresnelIntegral(Real x) const;

				const Parameters k = LinkwitzRiley::DefaultFM() * PI_2 / SPEED_OF_SOUND;	// Wave numbers for calculating UTD gains
				const std::array<Complex, 4> E = InitE();									// Coefficients for the UTD calculation

				LinkwitzRiley lrFilter;		// Linkwitz Riley filterbank
			};

			//////////////////// BTM class ////////////////////

			
			/**
			* @brief Class that implements the BTMS model (Biot-Tolstoy-Medwin-Svensson)
			* 
			* @remark Based after An analytic secondary source model of edge diffraction impulse responses. Svensson U et al. 1999 10.1121/1.428071,
			* Ported from Edge diffraction Matlab toolbox (EDtoolbox) by upsvensson https://github.com/upsvensson/Edge-diffraction-Matlab-toolbox
			*/
			class BTM : public Model
			{
				using Parameters = Coefficients<std::array<Real, 4>>; // Parameters type that stores 4 values

				/**
				* @brief Struct that stores the limits for the integral calculation
				*/
				struct IntegralLimits
				{
					Real p, m;
					IntegralLimits(Real p = 0.0, Real m = 0.0) : p(p), m(m) {}
				};

				/**
				* @brief Struct that calculates parameters used in the BTMS model
				*/
				struct Constants
				{
					Real dSSq{ 0.0 }, dRSq{ 0.0 };	// dS * dS and dR * dR
					Real rSSq{ 0.0 }, rRSq{ 0.0 };	// rS * rS and rR * rR
					Real rr{ 0.0 };					// rS * rR
					Real R0{ 0.0 }, R0Sq{ 0.0 };	// dS + dR and R0 * R0

					Real zSRel{ 0.0 }, zRRel{ 0.0 };	// zS - zA and zR - zA
					Real dz{ 0.0 }, dzSq{ 0.0 };		// zS - zR (dZ) and dZ * dZ

					Real v{ 0.0 }, vSq{ 0.0 };			// Exterioir wedge index (v) and v * v
					Real edgeHi{ 0.0 }, edgeLo{ 0.0 };	// Length of each edge half from apex

					Parameters theta{ 0.0 }, thetaSq{ 0.0 }, vTheta{ 0.0 };			// PI plus/minus (thetaR plus/minus thetaS), v * theta
					Parameters sinTheta{ 0.0 }, cosTheta{ 0.0 }, absTheta{ 0.0 };	// sin(v * theta), cos(v * theta) and abs(v * theta)
					
					Real zRange{ 0.0 }, zRangeApex{ 0.0 };	// z length of first sample
					bool splitIntegral{ true };				// True if first sample is split into two parts, false otherwise
					
					Real rho{ 0.0 }, rhoSq{ 0.0 };			// rR / rS, rho * rho
					Real rhoOne{ 0.0 }, rhoOneSq{ 0.0 };	// rho * rho, rho + 1.0 and (rho + 1.0) * (rho + 1.0)
					Real R0rho{ 0.0 };						// R0 * rho
					
					Real factor{ 0.0 };		// Commonly used factor

					Constants(const Path& path, const Real samplesPerMetre)
					{
						dSSq = path.sData.d * path.sData.d;
						dRSq = path.rData.d * path.rData.d;
						rSSq = path.sData.r * path.sData.r;
						rRSq = path.rData.r * path.rData.r;
						rr = path.sData.r * path.rData.r;
						R0 = path.sData.d + path.rData.d;
						R0Sq = R0 * R0;

						zSRel = path.sData.z - path.zA;
						zRRel = path.rData.z - path.zA;
						dz = zSRel - zRRel;
						dzSq = dz * dz;

						v = PI_1 / path.wData.t;
						vSq = v * v;
						edgeHi = path.wData.z - path.zA;
						edgeLo = -path.zA;

						Real plus = path.sData.t + path.rData.t;
						Real minus = path.sData.t - path.rData.t;
						theta = Parameters({ PI_1 + plus, PI_1 + minus, PI_1 - minus, PI_1 - plus });
						vTheta = v * theta;
						thetaSq = theta * theta;
						absTheta = Abs(vTheta);
						sinTheta = Sin(vTheta);
						cosTheta = Cos(vTheta);

						int n0 = (int)round(samplesPerMetre * R0);
						Real x = (n0 + 0.5) / samplesPerMetre;
						Real xSq = x * x;
						Real MSq = rSSq + zSRel * zSRel;
						Real LSq = rRSq + zRRel * zRRel;
						Real K = MSq - LSq - xSq;
						Real denom = dzSq - xSq;
						Real a = (2.0 * xSq * zRRel - K * dz) / denom;
						Real b = ((K * K / 4) - xSq * LSq) / denom;
						zRangeApex = -a / 2.0 + sqrt(a * a / 4.0 - b);
						zRange = 0.1 * std::min(path.sData.r, path.rData.r);

						if (zRange > zRangeApex)
						{
							zRange = abs(zRangeApex);
							splitIntegral = false;
						}

						rho = path.rData.r / path.sData.r;
						rhoSq = rho * rho;
						rhoOne = rho + 1.0;
						rhoOneSq = rhoOne * rhoOne;
						R0rho = R0 * rho;

						Real sinPsi = (path.sData.r + path.rData.r) / R0;
						factor = rhoOneSq * sinPsi * sinPsi - 2.0 * rho;
					}
				};

			public:
				/**
				* @brief Constructor that initialises the BTM model with a given path and sample rate
				* 
				* @param path The path to set the target parameters from
				* @param fs The sample rate for calculating BTM response
				*/
				BTM(const Path& path, int fs) : Model(), samplesPerMetre(fs* INV_SPEED_OF_SOUND), firFilter(CalculateBTM(path), maxIrLength)
				{
					isInitialised.store(true);
				};

				/**
				* @brief Default deconstructor
				*/
				~BTM() {};

				/**
				* @brief Set the target impulse response based on the given path
				*/
				void SetTargetParameters(const Path& path) override;

				/**
				* @brief Processes the input audio buffer and applies the FIR filter
				* 
				* @param inBuffer The input audio buffer
				* @param outBuffer The output audio buffer to write to
				* @param lerpFactor The lerp factor for interpolation
				*/
				void ProcessAudio(const Buffer& inBuffer, Buffer& outBuffer, const Real lerpFactor) override;

			private:
				/**
				* @brief Calculates the impulse response for the BTM model based on the given path
				* 
				* @param path The path to calculate the impulse response for
				*/
				Buffer CalculateBTM(const Path& path);

				/**
				* @brief Analytical solution for the first sample in skew case (i.e rS != rS and zS != zR)
				*
				* @param path The path to calculate the first sample for
				* @param constants The constants used in the BTMS calculation
				* @return The value for the first sample
				*/
				Real SkewCase(const Path& path, const Constants& constants);

				/**
				* @brief Analytical solution for the first sample in non skew case (i.e rS == rS or zS == zR)
				*
				* @param path The path to calculate the first sample for
				* @param constants The constants used in the BTMS calculation
				* @return The value for the first sample
				*/
				Real NonSkewCase(const Path& path, const Constants& constants);

				/**
				* @brief Calculate the sample value for a given index n
				* 
				* @param n The index of the sample to calculate
				* @param constants The constants used in the BTMS calculation
				*/
				Real CalculateSample(int n, const Constants& constants);

				/**
				* @brief Calculate the limits for the integral calculation
				* 
				* @param delta The delta value for the integral calculation
				* @param constants The constants used in the BTMS calculation
				*/
				IntegralLimits CalculateLimits(Real delta, const Constants& constants);
				
				/**
				* @brief Calculates adaptive Simpson quadrature
				* 
				* @param x1 lower limit
				* @param x3 upper limit
				* @param y1 value at x1
				* @param y2 value at x2
				* @param y3 value at x3
				* @param constants The constants used in the BTMS calculation
				* @return The value of the integral
				*/
				Real QuadStep(Real x1, Real x3, Real y1, Real y2, Real y3, const Constants& constants);
				
				/**
				* @brief Calculates the integral value for a given range using Quadstep simpson's rule
				* 
				* @param zn1 The lower limit of the integral
				* @param zn2 The upper limit of the integral
				* @param constants The constants used in the BTMS calculation
				*/
				Real CalculateIntegral(Real zn1, Real zn2, const Constants& constants);

				/**
				* @brief Calculates the integrand for the integral calculation
				* 
				* @param z The z value to calculate the integrand for
				* @param constants The constants used in the BTMS calculation
				*/
				Real CalculateIntegrand(Real z, const Constants& constants);

				Real samplesPerMetre;		// Samples per metre based on the sample rate
				Path lastPath;				// Previous path used to calculate the impulse response

				static constexpr size_t maxIrLength = 2048;		// Maximum length of the impulse response
				FIRFilter firFilter;							// FIRFilter
			};
		}
	}
}

#endif