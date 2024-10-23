/*
* @class FDN, Channel
*
* @brief Declaration of FDN and Channel classes
*
*/

// C++ headers
#include <mutex>
//#include <xmmintrin.h>
#include <cmath>

// Spatialiser headers
#include "Spatialiser/FDN.h"

// Unity headers
#include "Unity/UnityInterface.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{

		//////////////////// FDN Channel class ////////////////////

		Channel::Channel(const Config& config) : mT(1.0 / config.fs), mConfig(config), mAbsorptionFilter(mConfig.frequencyBands, mConfig.Q, mConfig.fs), idx(0)
		{
			mBufferMutex = std::make_shared<std::mutex>();
			SetDelay();
			SetAbsorption();
		}

		Channel::Channel(Real t, const Coefficients& T60, const Config& config) : mT(t), mConfig(config), mAbsorptionFilter(mConfig.frequencyBands, mConfig.Q, mConfig.fs), idx(0)
		{
			mBufferMutex = std::make_shared<std::mutex>();
			SetDelay();
			SetAbsorption(T60);
		}

		void Channel::SetParameters(const Coefficients& T60, const Real t)
		{
			SetDelay(t);
			if (T60 > 0.0)
				SetAbsorption(T60);
			else
				SetAbsorption();
		}

		void Channel::SetAbsorption()
		{
			Coefficients g = Coefficients(mConfig.frequencyBands.Length(), 1.0);
			mAbsorptionFilter.InitParameters(g);
		}

		void Channel::SetAbsorption(const Coefficients& T60)
		{
			Coefficients g = CalcGain(T60);
			mAbsorptionFilter.InitParameters(g);
		}

		void Channel::UpdateAbsorption(const Coefficients& T60)
		{
			Coefficients g = CalcGain(T60);
			mAbsorptionFilter.SetGain(g);
		}

		void Channel::SetDelay()
		{
			std::lock_guard<std::mutex> lock(*mBufferMutex);
			mBuffer.ResizeBuffer(round(mT * mConfig.fs));
		}

		Real Channel::GetOutput(const Real input)
		{
			std::lock_guard<std::mutex> lock(*mBufferMutex);

			if (idx >= mBuffer.Length())
				idx = 0;
			Real out = mAbsorptionFilter.GetOutput(mBuffer[idx]);
			mAbsorptionFilter.UpdateParameters(mConfig.lerpFactor);
			mBuffer[idx] = input;
			++idx;
			return out;
		}

		//////////////////// FDN class ////////////////////

		FDN::FDN(const Config& config) : mConfig(config), x(config.numFDNChannels), y(config.numFDNChannels), mat(config.numFDNChannels, config.numFDNChannels)
		{
			mChannels = std::vector<Channel>(mConfig.numFDNChannels, Channel(mConfig));
			SetFDNModel(FDNMatrix::householder);
		}

		FDN::FDN(const Coefficients& T60, const vec& dimensions, const Config& config) : mConfig(config), x(config.numFDNChannels), y(config.numFDNChannels), mat(config.numFDNChannels, config.numFDNChannels)
		{
			vec t = CalculateTimeDelay(dimensions);
			mChannels.reserve(mConfig.numFDNChannels);
			for (int i = 0; i < mConfig.numFDNChannels; i++)
				mChannels.push_back(Channel(t[i], T60, mConfig));
			SetFDNModel(FDNMatrix::householder);
		}

		void FDN::UpdateT60(const Coefficients& T60)
		{
			for (int i = 0; i < mConfig.numFDNChannels; i++)
				mChannels[i].UpdateAbsorption(T60);
		}

		void FDN::SetParameters(const Coefficients& T60, const vec& dimensions)
		{
			vec t = CalculateTimeDelay(dimensions);
			for (int i = 0; i < mConfig.numFDNChannels; i++)
				mChannels[i].SetParameters(T60, t[i]);
		}

		vec FDN::CalculateTimeDelay(const vec& dimensions)
		{
			vec t = vec(mConfig.numFDNChannels);
			if (dimensions.Rows() > 0)
			{
				Real idx = static_cast<Real>(mConfig.numFDNChannels) / static_cast<Real>(dimensions.Rows());
				//assert(t.Rows() == config.numFDNChannels);
				//assert(dimensions.Rows() <= config.numFDNChannels);
				//assert(idx == floor(idx)); // length of dimensions must be a multiple of mNumChannels

				t.RandomUniformDistribution(-0.1, 0.1f);
				t *= dimensions.Mean();

				int k = 0;
				for (int j = 0; j < mConfig.numFDNChannels / idx; ++j)
				{
					for (int i = 0; i < idx; ++i)
					{
						t[k] += dimensions[j];
						++k;
					}
				}
				t *= INV_SPEED_OF_SOUND;
				t.Max(0.0);
			}
			return t;
		}

		void FDN::InitRandomOrthogonal()
		{
			vec vector = vec(mConfig.numFDNChannels);
			vector.RandomUniformDistribution(-1.0, 1.0);
			vector.Normalise();
			mat.AddColumn(vector.GetColumn(0), 0);

			Real tol = 0.000001;
			for (int j = 1; j < mConfig.numFDNChannels; ++j)
			{
				Real norm = 0;
				while (norm < tol)
				{
					vector.RandomUniformDistribution(-1.0, 1.0);

					matrix section = matrix(mConfig.numFDNChannels, j);

					for (int i = 0; i < mConfig.numFDNChannels; ++i)
					{
						for (int k = 0; k < j; k++)
							section[i][k] = mat.GetEntry(i, k);
					}

					vector -= section * (section.Transpose() * vector);
					norm = vector.CalculateNormal();
				}
				vector /= norm;
				mat.AddColumn(vector.GetColumn(0), j);
			}
		}

		//void FDN::InitMatrix()
		//{
		//	vec vector = vec(mConfig.numFDNChannels);

		//	const int numCh = 12;

		//	std::vector<Real> col = std::vector<Real>({ 0.190688769837721, 0.385638500937477, -0.214721607924256, -0.223163172256605, 0.416580913100255, 0.134080009434784, -0.500189794156601, 0.383362598914846, 0.0964280106643111, -0.181167057154258, 0.283488160223467, 0.0886247705279117 });
		//	houseMat.AddColumn(col, 0);

		//	std::vector<Real> col0 = std::vector<Real>({ 0.251288080910173, 0.463081025830656, 0.123001447429897, 0.111209155896334, 0.283637518863125, 0.570862560700583, 0.277922724930659, -0.0140354015504064, 0.367357631085153, 0.256510104845225, -0.0578512136087400, -0.0839360870952322 });
		//	std::vector<Real> col1 = std::vector<Real>({ 0.324000644136978, 0.0771150039207005, 0.221186170062238, -0.218478347921605, 0.0930050966965055, -0.236068488842949, -0.162162903275363, 0.252168440461209, -0.0636391434234939, 0.293980204799346, 0.680984148140615, -0.289729232256578 });
		//	std::vector<Real> col2 = std::vector<Real>({ -0.297828765804106, 0.176011218147238, 0.242348095565290, 0.327022627771562, 0.0858729131180891, -0.00706759064976401, 0.331432781263459, 0.399747334659180, -0.601847346124569, 0.0558459994652321, -0.118327972777587, -0.241311909991920 });
		//	std::vector<Real> col3 = std::vector<Real>({ 0.330055950901652, -0.216869057453229, -0.111159901172237, -0.353054687262316, -0.173534253414560, 0.259129139763574, 0.165010459499892, -0.207881337681749, -0.262200967910925, -0.176497541439552, -0.197385483265019, -0.632818960029143 });
		//	std::vector<Real> col4 = std::vector<Real>({ 0.105680958925820, -0.0380855664321653, 0.145415322011594, -0.0618778110602707, 0.141599428075859, 0.537364009938170, -0.599570520866479, 0.0282576891073156, -0.455206204497853, -0.0325058263167070, -0.00379402734497303, 0.291597409929032 });
		//	std::vector<Real> col5 = std::vector<Real>({ -0.321339967863874, 0.268880543333346, -0.314042404681992, -0.388009804647359, -0.494077032390177, 0.171201662008169, -0.140221504694795, 0.370915492213603, 0.0648917260264280, 0.357520602620454, -0.106791399615939, -0.0342172739941697 });
		//	std::vector<Real> col6 = std::vector<Real>({ -0.176855953032873, 0.202554816161202, 0.196535383754806, 0.118413872277773, -0.331732594692273, -0.0227115087022323, 0.0106217702022385, -0.730451358081194, -0.230102744089435, 0.392275653584726, 0.154866614612900, -0.000225882951458356 });
		//	std::vector<Real> col7 = std::vector<Real>({ 0.0374320952013009, 0.405874404532155, -0.482292752344398, 0.380882230633527, 0.164179460091712, -0.176380374350674, -0.466468700072716, -0.101540906997748, -0.0243014116201423, -0.101601368322497, -0.0677289874457483, -0.393342198997892 });
		//	std::vector<Real> col8 = std::vector<Real>({ 0.365291904136971, 0.235189691239062, -0.256275018498463, -0.320315736415488, 0.274977779402982, -0.346141648014959, 0.188311867284220, -0.0674697172225185, -0.341404349595036, 0.271695641064810, -0.308151515259276, 0.349564626920972 });
		//	std::vector<Real> col9 = std::vector<Real>({ 0.371185750860281, -0.296872329344590, -0.442052496522062, 0.490616458805380, -0.314888739469575, 0.177076342712406, 0.191703210839261, 0.137325777767009, -0.129828962753109, 0.222800036286988, 0.227081677748887, 0.183826263925673 });
		//	std::vector<Real> col10 = std::vector<Real>({ -0.273375520667077, 0.224828294615083, -0.385627713105243, -0.220761398976612, 0.166485158095585, 0.199525454641433, 0.300012161100184, -0.131537924462519, -0.175763437367472, -0.397051080461701, 0.541602429645960, 0.143390929776798 });
		//	std::vector<Real> col11 = std::vector<Real>({ 0.375740251310707, 0.477590019868876, 0.253103669081037, 0.0528437051430278, -0.519386743020745, -0.102534518236492, 0.0177337050031824, 0.101203773583646, -0.0276065775818014, -0.489202895520961, -0.0102455189239106, 0.181256560942768 });
		//	mat.AddRow(col0, 0);
		//	mat.AddRow(col1, 1);
		//	mat.AddRow(col2, 2);
		//	mat.AddRow(col3, 3);
		//	mat.AddRow(col4, 4);
		//	mat.AddRow(col5, 5);
		//	mat.AddRow(col6, 6);
		//	mat.AddRow(col7, 7);
		//	mat.AddRow(col8, 8);
		//	mat.AddRow(col9, 9);
		//	mat.AddRow(col10, 10);
		//	mat.AddRow(col11, 11);

		//	/*for (int i = 0; i < mNumChannels; i++)
		//	{
		//		mat.AddEntry(1.0, i, i);
		//	}*/

		//	/*vector.RandomUniformDistribution(-1.0, 1.0);
		//	vector.Normalise();
		//	mat.AddColumn(vector, 0);

		//	Real tol = 0.000001;
		//	for (int j = 1; j < mNumChannels; j++)
		//	{
		//		Real norm = 0;
		//		while (norm < tol)
		//		{
		//			vector.RandomUniformDistribution(-1.0, 1.0);

		//			matrix section = matrix(mNumChannels, j);

		//			for (int i = 0; i < mNumChannels; i++)
		//			{
		//				for (int k = 0; k < j; k++)
		//				{
		//					section.AddEntry(mat.GetEntry(i, k), i, k);
		//				}
		//			}

		//			vector -= section * (section.Transpose() * vector);
		//			norm = vector.CalculateNormal();
		//		}
		//		vector /= norm;
		//		mat.AddColumn(vector, j);
		//	}*/
		//}

		void FDN::ProcessOutput(const std::vector<Real>& data, const Real gain)
		{
#ifdef PROFILE_DETAILED
			BeginFDNChannel();
#endif

			int i = 0;
			for (auto& channel : mChannels)
			{
				if (isnan(x[i]))
					Debug::Log("X was nan", Colour::Red);
				y[i] = channel.GetOutput(x[i] + data[i]);
				if (isnan(y[i]))
					Debug::Log("Y was nan", Colour::Red);
				++i;
			}
			y *= gain;
#ifdef PROFILE_DETAILED
			EndFDNChannel();
			BeginFDNMatrix();
#endif
			ProcessMatrix();
#ifdef PROFILE_DETAILED
			EndFDNMatrix();
#endif
		}
	}
}
