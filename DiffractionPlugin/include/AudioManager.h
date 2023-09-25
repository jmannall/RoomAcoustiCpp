#pragma once

#include "UnityGAPlugin.h"

#pragma region Buffer
class Buffer
{
public:
	Buffer();
	Buffer(int n);
	~Buffer() {};

	inline float& operator[](const int& i) { return mBuffer[i]; };

	void ResetBuffer();
	void ResizeBuffer(size_t numSamples);
	size_t Length() { return mBuffer.size(); }
	bool Valid();

	std::vector<double> GetBuffer() { std::vector<double> dBuffer(mBuffer.begin(), mBuffer.end()); return dBuffer; }

private:
	void InitialiseBuffer(int n);

	std::vector<float> mBuffer;
};

bool BuffersEqual(Buffer x, Buffer y);

#pragma endregion

#pragma region Source
class Source
{
public:
	Source() : position(vec3(0, 0, 0)) {};
	Source(float x, float y, float z) : position(vec3(x, y, z)) {};
	Source(vec3 position_) : position(position_) {};
	~Source() {};

	vec3 position;
private:
};

using Listener = Source;
#pragma endregion

#pragma region FIRFilter
class FIRFilter
{
public:
	FIRFilter(Buffer ir) : count(0), x() { SetImpulseResponse(ir); };
	~FIRFilter() {};

	float GetOutput(float input);
	void SetImpulseResponse(Buffer& ir) { mIr = ir; irLen = mIr.Length(); }

private:
	Buffer mIr;
	Buffer x;
	int count;
	size_t irLen;
};
#pragma endregion

#pragma region IIRFilter
struct FilterCoefficients
{
	Buffer a;
	Buffer b;
	FilterCoefficients(int n) : a(n), b(n + 1) {};
};

class IIRFilter
{
public:
	IIRFilter(int _order, int fs) : order(_order), T(1.0f / (float)fs), b(order + 1), a(order + 1), x(order + 1), y(order + 1) {};
	~IIRFilter() {};

	float GetOutput(float input);
	void SetT(int fs);

protected:
	int order;
	float T;
	Buffer b;
	Buffer a;
	Buffer x;
	Buffer y;
};

class HighShelf : public IIRFilter
{
public:
	HighShelf() : IIRFilter(1, 48000) {};
	HighShelf(int fs) : IIRFilter(1, fs) {};
	HighShelf(float fc, float g, int fs) : IIRFilter(1, fs) { UpdateParameters(fc, g); };
	void UpdateParameters(float fc, float g);

private:
};

class LowPass : public IIRFilter
{
public:
	LowPass() : IIRFilter(1, 48000) {};
	LowPass(int fs) : IIRFilter(1, fs) {};
	LowPass(float fc, int fs) : IIRFilter(1, fs) { UpdateParameters(fc); };
	void UpdateParameters(float fc);

private:
};

struct TransDF2Parameters
{
	float z[2];
	float p[2];
	float k;
	TransDF2Parameters() : z{ 0.25f, -0.99f }, p{ 0.99f, -0.25f }, k(0.0f) {};
	TransDF2Parameters(float _z, float _p, float _k) : z{ _z, _z }, p{ _p, _p }, k(_k) {};
};

enum class FilterShape
{
	lpf,
	hpf
};

class TransDF2 : public IIRFilter
{
public:
	TransDF2() : IIRFilter(2, 48000) { a[0] = 1.0f; };
	TransDF2(int fs) : IIRFilter(2, fs) { a[0] = 1.0f; };
	TransDF2(TransDF2Parameters zpk, int fs) : IIRFilter(2, fs) { a[0] = 1.0f; UpdateParameters(zpk); };
	TransDF2(float fc, FilterShape shape, int fs) : IIRFilter(2, fs) { a[0] = 1.0f; UpdateParameters(fc, shape); };
	void UpdateParameters(TransDF2Parameters zpk);
	void UpdateParameters(float fc, FilterShape shape);

private:
	void UpdateLPF(float fc);
	void UpdateHPF(float fc);
};

class LinkwitzRiley
{
public:
	LinkwitzRiley(int fs);
	LinkwitzRiley(float fc0, float fc1, float fc2, int fs);
	~LinkwitzRiley() {};

	float GetOutput(float input);
	void UpdateParameters(float gain[]);

	float fm[4];

private:
	void InitFilters(int fs);
	void CalcFM();

	float fc[3];
	float g[4];

	TransDF2 filters[20];
};
#pragma endregion
