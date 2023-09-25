
#include "DiffractionGeometry.h"

float Test() { return 1.0f; }

Wedge::Wedge() : zW(0.0f), mBase(vec3(0, 0, 0)), mTop(vec3(0, 1, 0)), mFaceNormals{vec3(1, 0, 0), vec3(0, 0, 1)}
{
	InitWedge();
}

Wedge::Wedge(vec3 base, vec3 top, vec3* faceNormals) : zW(0.0f), mBase(base),
mTop(top), mFaceNormals{ faceNormals[0], faceNormals[1] }
{
	InitWedge();
}

void Wedge::InitWedge()
{
	mEdgeVector = UnitVector(mTop - mBase);
	mEdgeNormal = UnitVector(mFaceNormals[0] + mFaceNormals[1]);

	if (Cross(mFaceNormals[0], mFaceNormals[1]) == mEdgeVector)
		t = 2 * PI - acosf(Dot(mFaceNormals[0], mFaceNormals[1]));
	else
		t = acosf(Dot(mFaceNormals[0], mFaceNormals[1]));

	UpdateEdgeLength();
}

DiffractionPath::DiffractionPath(Source* source, Receiver* receiver, Wedge* wedge) : valid(false), inShadow(false), mWedge(wedge)
{
	UpdateParameters(source, receiver, wedge);
}

void DiffractionPath::UpdateParameters(Source* source, Receiver* receiver, Wedge* wedge)
{
	sData.point = source->position;
	rData.point = receiver->position;
	mWedge = wedge;
	UpdateParameters();
}

void DiffractionPath::UpdateParameters(Source* source, Receiver* receiver)
{
	sData.point = source->position;
	rData.point = receiver->position;
	UpdateParameters();
}

void DiffractionPath::UpdateParameters(Receiver* receiver)
{
	rData.point = receiver->position;
	CalcR(&rData);
	CalcZ(&rData);
	CalcT(&rData);
	CorrectT();
	CalcApex();
	CalcD();
	ValidPath();
}

void DiffractionPath::UpdateParameters()
{
	UpdateWData();
	CalcR();
	CalcZ();
	CalcT();
	CalcApex();
	CalcD();
	UpdateBaMa();
	ValidPath();
}

void DiffractionPath::UpdateWData()
{
	wData.t = mWedge->t;
	wData.z = mWedge->zW;
}

void DiffractionPath::UpdateBaMa()
{
	bA = fabs(rData.t - sData.t);
	mA = fmin(sData.t, rData.t);
	if (bA > PI)
		inShadow = true;
	else
		inShadow = false;
}

void DiffractionPath::ValidPath()
{
	if ((zA < 0) | (zA > wData.z))
		valid = false;
	if ((sData.t > wData.t) | (rData.t > wData.t))
		valid = false;
	valid = true;
}

void DiffractionPath::CalcR()
{
	CalcR(&sData);
	CalcR(&rData);
}

void DiffractionPath::CalcR(SRData* data)
{
	data->r = (Cross(mWedge->GetAP(data->point), mWedge->mEdgeVector)).Length();
}

void DiffractionPath::CalcZ()
{
	CalcZ(&sData);
	CalcZ(&rData);
}

void DiffractionPath::CalcZ(SRData* data)
{
	vec3 AP = mWedge->GetAP(data->point);
	data->z = AP.Length() * Dot(UnitVector(AP), mWedge->mEdgeVector);
}

void DiffractionPath::CalcT()
{
	CalcT(&sData);
	CalcT(&rData);

	CorrectT();
}

void DiffractionPath::CorrectT()
{
	float halfThetaW = wData.t / 2;
	if (sData.rot == rData.rot)
	{
		if (sData.t > rData.t)
		{
			sData.t = halfThetaW + sData.t;
			rData.t = halfThetaW + rData.t;
		}
		else
		{
			sData.t = halfThetaW - sData.t;
			rData.t = halfThetaW - rData.t;
		}
	}
	else
	{
		sData.t = halfThetaW - sData.t;
		rData.t = halfThetaW + rData.t;
	}
}

void DiffractionPath::CalcT(SRData* data)
{
	vec3 k = UnitVector(data->point - mWedge->GetEdgeCoord(data->z));
	vec3 edgeNorm = mWedge->mEdgeNormal;
	data->t = acosf(Dot(k, edgeNorm));
	data->rot = signbit(Dot(Cross(k, edgeNorm), mWedge->mEdgeVector));
}

void DiffractionPath::CalcApex()
{
	float dZ = fabsf(rData.z - sData.z) * sData.r / (sData.r + rData.r);
	if (sData.z > rData.z)
		zA = sData.z - dZ;
	else
		zA = sData.z + dZ;
	phi = atanf(sData.r / dZ);
}

void DiffractionPath::CalcD()
{
	CalcD(&sData);
	CalcD(&rData);
}

void DiffractionPath::CalcD(SRData* data)
{
	vec3 apex = mWedge->GetEdgeCoord(zA);
	data->d = (data->point - mWedge->GetEdgeCoord(zA)).Length();
}

float DiffractionPath::GetD(float z)
{
	return sqrtf(powf(sData.r, 2) + powf(z - sData.z, 2)) + sqrtf(powf(rData.r, 2) + powf(z - rData.z, 2));
}

float DiffractionPath::GetMaxD()
{
	return std::max(GetD(0), GetD(wData.z));
}