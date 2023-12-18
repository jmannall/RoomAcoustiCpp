
#include "Spatialiser/FDN.h"
#include <xmmintrin.h>

using namespace Spatialiser;

Channel::Channel(int fs) : mT(1.0f / fs), sampleRate(fs), mAbsorptionFilter(4, fs), mAirAbsorption(14000, fs), idx(0)
{
	SetDelay();
	SetAbsorption({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
}

Channel::Channel(float t, const FrequencyDependence& T60, int fs) : mT(t), sampleRate(fs), mAbsorptionFilter(4, fs), mAirAbsorption(14000, fs), idx(0)
{
	SetDelay();
	SetAbsorption(T60);
}

void Channel::SetParameters(const FrequencyDependence& T60, const float& t)
{
	SetDelay(t);
	SetAbsorption(T60);
}

void Channel::SetAbsorption(const FrequencyDependence& T60)
{
	float g[5];
	float g2[5];
	float t[5];

	float delay = mT * 44100;

	T60.GetValues(&t[0]);
	for (int i = 0; i < 5; i++)
	{
		g[i] = powf(10, -3.0f * mT / t[i]); // 20 * log10(H(f)) = -60 * t / t60(f);
		g2[i] = powf(10, -3.0f / 44100 / t[i] * delay);
	}
	SetAbsorption(g);
}

void Channel::SetAbsorption(float g[])
{
	float fc[] = { 250, 500, 1000, 2000, 4000 };
	mAbsorptionFilter.UpdateParameters(fc, g);
}

void Channel::SetDelay()
{
	mDelay = round(mT * sampleRate);
	mBuffer.ResizeBuffer(mDelay);
}

float Channel::GetOutput(const float input)
{
	float out = mAirAbsorption.GetOutput(mAbsorptionFilter.GetOutput(mBuffer[idx]));
	mBuffer[idx] = input;
	idx++;
	idx = idx % mDelay;
	return out;
}

FDN::FDN(size_t numChannels, int fs) : mNumChannels(numChannels), x(mNumChannels), y(mNumChannels), mat(mNumChannels, mNumChannels)
{
	mChannels.reserve(mNumChannels);
	std::fill_n(std::back_inserter(mChannels), mNumChannels, Channel(fs));
	InitMatrix();
}

FDN::FDN(const FrequencyDependence& T60, const vec& dimensions, size_t numChannels, int fs) : mNumChannels(numChannels), x(mNumChannels), y(mNumChannels), mat(mNumChannels, mNumChannels)
{
	vec t = vec(mNumChannels);
	CalculateTimeDelay(dimensions, t);
	mChannels.reserve(mNumChannels);
	for (int i = 0; i < mNumChannels; i++)
	{
		mChannels.push_back(Channel(t[i], T60, fs));
	}
	InitMatrix();
}

void FDN::SetParameters(const FrequencyDependence& T60, const vec& dimensions)
{
	vec t = vec(mNumChannels);
	CalculateTimeDelay(dimensions, t);
	for (int i = 0; i < mNumChannels; i++)
	{
		mChannels[i].SetParameters(T60, t[i]);
	}
}

void FDN::CalculateTimeDelay(const vec& dimensions, vec& t)
{
	float idx = mNumChannels / dimensions.Rows();
	assert(t.Rows() == mNumChannels);
	assert(dimensions.Rows() <= mNumChannels);
	assert(idx == floor(idx)); // length of dimensions must be a multiple of mNumChannels

	t.RandomUniformDistribution(-0.1f, 0.1f);
	t *= dimensions.Mean();

	int k = 0;
	for (int j = 0; j < mNumChannels / idx; j++)
	{
		for (int i = 0; i < idx; i++)
		{
			t[k++] += dimensions[j];
		}
	}
	t *= INV_SPEED_OF_SOUND;
}

void FDN::InitMatrix()
{
	vec vector = vec(mNumChannels);

	const int numCh = 12;
	float col0[numCh] = { 0.251288080910173f, 0.463081025830656f, 0.123001447429897f, 0.111209155896334f, 0.283637518863125f, 0.570862560700583f, 0.277922724930659f, -0.0140354015504064f, 0.367357631085153f, 0.256510104845225f, -0.0578512136087400f, -0.0839360870952322f };
	float col1[numCh] = { 0.324000644136978f, 0.0771150039207005f, 0.221186170062238f, -0.218478347921605f, 0.0930050966965055f, -0.236068488842949f, -0.162162903275363f, 0.252168440461209f, -0.0636391434234939f, 0.293980204799346f, 0.680984148140615f, -0.289729232256578f };
	float col2[numCh] = { -0.297828765804106f, 0.176011218147238f, 0.242348095565290f, 0.327022627771562f, 0.0858729131180891f, -0.00706759064976401f, 0.331432781263459f, 0.399747334659180f, -0.601847346124569f, 0.0558459994652321f, -0.118327972777587f, -0.241311909991920f };
	float col3[numCh] = { 0.330055950901652f, -0.216869057453229f, -0.111159901172237f, -0.353054687262316f, -0.173534253414560f, 0.259129139763574f, 0.165010459499892f, -0.207881337681749f, -0.262200967910925f, -0.176497541439552f, -0.197385483265019f, -0.632818960029143f };
	float col4[numCh] = { 0.105680958925820f, -0.0380855664321653f, 0.145415322011594f, -0.0618778110602707f, 0.141599428075859f, 0.537364009938170f, -0.599570520866479f, 0.0282576891073156f, -0.455206204497853f, -0.0325058263167070f, -0.00379402734497303f, 0.291597409929032f };
	float col5[numCh] = { -0.321339967863874f, 0.268880543333346f, -0.314042404681992f, -0.388009804647359f, -0.494077032390177f, 0.171201662008169f, -0.140221504694795f, 0.370915492213603f, 0.0648917260264280f, 0.357520602620454f, -0.106791399615939f, -0.0342172739941697f };
	float col6[numCh] = { -0.176855953032873f, 0.202554816161202f, 0.196535383754806f, 0.118413872277773f, -0.331732594692273f, -0.0227115087022323f, 0.0106217702022385f, -0.730451358081194f, -0.230102744089435f, 0.392275653584726f, 0.154866614612900f, -0.000225882951458356f };
	float col7[numCh] = { 0.0374320952013009f, 0.405874404532155f, -0.482292752344398f, 0.380882230633527f, 0.164179460091712f, -0.176380374350674f, -0.466468700072716f, -0.101540906997748f, -0.0243014116201423f, -0.101601368322497f, -0.0677289874457483f, -0.393342198997892f };
	float col8[numCh] = { 0.365291904136971f, 0.235189691239062f, -0.256275018498463f, -0.320315736415488f, 0.274977779402982f, -0.346141648014959f, 0.188311867284220f, -0.0674697172225185f, -0.341404349595036f, 0.271695641064810f, -0.308151515259276f, 0.349564626920972f };
	float col9[numCh] = { 0.371185750860281f, -0.296872329344590f, -0.442052496522062f, 0.490616458805380f, -0.314888739469575f, 0.177076342712406f, 0.191703210839261f, 0.137325777767009f, -0.129828962753109f, 0.222800036286988f, 0.227081677748887f, 0.183826263925673f };
	float col10[numCh] = { -0.273375520667077f, 0.224828294615083f, -0.385627713105243f, -0.220761398976612f, 0.166485158095585f, 0.199525454641433f, 0.300012161100184f, -0.131537924462519f, -0.175763437367472f, -0.397051080461701f, 0.541602429645960f, 0.143390929776798f };
	float col11[numCh] = { 0.375740251310707f, 0.477590019868876f, 0.253103669081037f, 0.0528437051430278f, -0.519386743020745f, -0.102534518236492f, 0.0177337050031824f, 0.101203773583646f, -0.0276065775818014f, -0.489202895520961f, -0.0102455189239106f, 0.181256560942768f };
	mat.AddRow(vec(&col0[0], numCh), 0);
	mat.AddRow(vec(&col1[0], numCh), 1);
	mat.AddRow(vec(&col2[0], numCh), 2);
	mat.AddRow(vec(&col3[0], numCh), 3);
	mat.AddRow(vec(&col4[0], numCh), 4);
	mat.AddRow(vec(&col5[0], numCh), 5);
	mat.AddRow(vec(&col6[0], numCh), 6);
	mat.AddRow(vec(&col7[0], numCh), 7);
	mat.AddRow(vec(&col8[0], numCh), 8);
	mat.AddRow(vec(&col9[0], numCh), 9);
	mat.AddRow(vec(&col10[0], numCh), 10);
	mat.AddRow(vec(&col11[0], numCh), 11);

	/*for (int i = 0; i < mNumChannels; i++)
	{
		mat.AddEntry(1.0f, i, i);
	}*/

	/*vector.RandomUniformDistribution(-1.0f, 1.0f);
	vector.Normalise();
	mat.AddColumn(vector, 0);

	float tol = 0.000001;
	for (int j = 1; j < mNumChannels; j++)
	{
		float norm = 0;
		while (norm < tol)
		{
			vector.RandomUniformDistribution(-1.0f, 1.0f);

			matrix section = matrix(mNumChannels, j);

			for (int i = 0; i < mNumChannels; i++)
			{
				for (int k = 0; k < j; k++)
				{
					section.AddEntry(mat.GetEntry(i, k), i, k);
				}
			}

			vector -= section * (section.Transpose() * vector);
			norm = vector.CalculateNormal();
		}
		vector /= norm;
		mat.AddColumn(vector, j);
	}*/
}

rowvec FDN::GetOutput(const float* data, bool valid)
{
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

	float output = 0;
	for (int i = 0; i < mNumChannels; i++)
	{
		y[i] = mChannels[i].GetOutput(x[i] + data[i]);
	}
	ProcessMatrix();
	return y;
}

