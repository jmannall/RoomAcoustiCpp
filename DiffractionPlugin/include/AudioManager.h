#pragma once

#ifndef AudioManager_h
#define AudioManager_h

#include <vector>
#include <stdlib.h>
#include "Definitions.h"
#include "Types.h"

//////////////////// Buffer ////////////////////

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

//class Source
//{
//public:
//	Source() : position(vec3(0, 0, 0)) {};
//	Source(float x, float y, float z) : position(vec3(x, y, z)) {};
//	Source(vec3 position_) : position(position_) {};
//	~Source() {};
//
//	vec3 position;
//private:
//};
//
//using Listener = Source;


//////////////////// FIR Filter ////////////////////

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


//struct FilterCoefficients
//{
//	Buffer a;
//	Buffer b;
//	FilterCoefficients(int n) : a(n), b(n + 1) {};
//};

//////////////////// IIR Filter ////////////////////

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

enum class FilterShape
{
	lpf,
	hpf,
	lbf,
	hbf
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

class TransDF2 : public IIRFilter
{
public:
	TransDF2() : IIRFilter(2, 48000) { a[0] = 1.0f; };
	TransDF2(int fs) : IIRFilter(2, fs) { a[0] = 1.0f; };
	TransDF2(TransDF2Parameters zpk, int fs) : IIRFilter(2, fs) { a[0] = 1.0f; UpdateParameters(zpk); };
	TransDF2(float fc, FilterShape shape, int fs) : IIRFilter(2, fs) { a[0] = 1.0f; UpdateParameters(fc, shape); };
	TransDF2(float fb, float g, int m, int M, FilterShape shape, int fs) : IIRFilter(2, fs) { a[0] = 1.0f; UpdateParameters(fb, g, m, M, shape); };
	void UpdateParameters(TransDF2Parameters zpk);
	void UpdateParameters(float fc, FilterShape shape);
	void UpdateParameters(float fb, float g, int m, int M, FilterShape shape);

private:
	void UpdateLPF(float fc);
	void UpdateHPF(float fc);
	void UpdateLBF(float fb, float g, int m, int M);
	void UpdateHBF(float fb, float g, int m, int M);
};

//////////////////// Filterbanks ////////////////////

class LinkwitzRiley
{
public:
	LinkwitzRiley(int fs);
	LinkwitzRiley(float fc0, float fc1, float fc2, int fs);
	~LinkwitzRiley() {};

	float GetOutput(const float input);
	void UpdateParameters(float gain[]);

	float fm[4];

private:
	void InitFilters(int fs);
	void CalcFM();

	float fc[3];
	float g[4];

	TransDF2 filters[20];
};

class BandPass
{
public:
	BandPass();
	BandPass(size_t order);
	BandPass(size_t order, int fs);
	BandPass(size_t order, FilterShape shape, float fb, float g, int fs);

	void InitFilters(int order, int fs);
	void UpdateParameters(float fb, float g, FilterShape shape);
	float GetOutput(const float input);

private:
	int M;
	size_t numFilters;
	std::vector<TransDF2> filters;
};

class ParametricEQ
{
public:
	ParametricEQ(size_t order);
	ParametricEQ(size_t order, int fs);
	ParametricEQ(size_t order, float fc[], float gain[], int fs);

	void UpdateParameters(const float fc[], float gain[]);
	float GetOutput(const float input);
private:
	void InitBands(int fs);

	size_t mOrder;
	size_t numFilters;
	BandPass bands[4];
	float mGain;
	float fb[4];
	float g[4];
};

#endif



