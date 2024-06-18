#pragma once

#include "SourceManager.h"
#include "UnityGAPlugin.h"

class Source;
using Receiver = Source;

float Test();

inline float Deg2Rad(const float degrees) { return degrees * (PI_1 / 180); }

inline float Rad2Deg(const float radians) { return radians * (180 / PI_1); }

struct WedgeData
{
	vec3 base;
	vec3 top;
	vec3* faceNormals;
	WedgeData() : base(vec3(0, 0, 0)), top(vec3(0, 1, 0)), faceNormals(nullptr) {}
	WedgeData(vec3 base_, vec3 top_, vec3* faceNormals_) : base(base_), top(top_), faceNormals(faceNormals_) {}
};

// faceNormals defined using right hand curl rule that rotates from plane 0 to plane 1 through the exterior wedge.
class Wedge
{
public:
	Wedge();
	Wedge(vec3 base, vec3 top, vec3* faceNormals);
	~Wedge() {};

	void InitWedge();
	void UpdateEdgeLength() { zW = (mTop - mBase).Length(); }
	vec3 GetAP(vec3 point) { return point - mBase; }
	vec3 GetEdgeCoord(float z) { return mBase + z * mEdgeVector; }
	float GetThetaW() { return Rad2Deg(t); }

	float t;
	float zW;
	vec3 mEdgeVector;
	vec3 mEdgeNormal;

private:
	vec3 mBase;
	vec3 mTop;
	vec3 mFaceNormals[2];
};

struct SRData
{
	vec3 point;
	float r;
	float z;
	float t;
	float d;
	bool rot;
};

struct WData
{
	float z;
	float t;
};


class DiffractionPath
{
public:
	DiffractionPath(Source* source, Receiver* receiver, Wedge* wedge);
	~DiffractionPath() {};

	void UpdateParameters(Source* source, Receiver* receiver, Wedge* wedge);
	void UpdateParameters(Source* source, Receiver* receiver);
	void UpdateParameters(Receiver* receiver);
	float GetD(float z);
	float GetMaxD();

	SRData sData;
	SRData rData;
	WData wData;
	float bA;
	float mA;
	float zA;
	float phi;
	bool valid;
	bool inShadow;

private:
	Wedge* mWedge;
	void UpdateParameters();
	void UpdateWData();
	void UpdateBaMa();
	void CalcR();
	void CalcR(SRData* data);
	void CalcZ();
	void CalcZ(SRData* data);
	void CalcT();
	void CalcT(SRData* data);
	void CorrectT();
	void CalcApex();
	void CalcD();
	void CalcD(SRData* data);
	
	void ValidPath();
};

