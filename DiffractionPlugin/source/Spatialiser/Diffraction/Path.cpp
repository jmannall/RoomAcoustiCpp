/*
*
*  \Diffraction path class
*
*/

// Common headers
#include "Common/Definitions.h"

// Spatialiser headers
#include "Spatialiser/Diffraction/Path.h"

namespace RAC
{
	namespace Spatialiser
	{
		namespace Diffraction
		{

			Path::Path(const vec3& source, const vec3& receiver, const Edge& edge) : zValid(false), sValid(false), rValid(false), valid(false), inShadow(false), mEdge(edge)
			{
				UpdateParameters(source, receiver);
			}

			void Path::UpdateParameters(const vec3& source, const vec3& receiver, const Edge& edge)
			{
				sData.point = source;
				rData.point = receiver;
				mEdge = edge;
				UpdateParameters();
			}

			void Path::UpdateParameters(const vec3& source, const vec3& receiver)
			{
				sData.point = source;
				rData.point = receiver;
				UpdateParameters();
			}

			void Path::UpdateParameters(const vec3& receiver)
			{
				rData.point = receiver;
				CalcR(&rData);
				CalcZ(&rData);
				CalcT(&rData);
				CorrectT();
				CalcApex();
				CalcD();
				ValidPath();
			}

			void Path::UpdateParameters()
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

			void Path::UpdateWData()
			{
				wData.t = mEdge.t;
				wData.z = mEdge.zW;
			}

			void Path::UpdateBaMa()
			{
				bA = fabs(rData.t - sData.t);
				mA = fmin(sData.t, rData.t);
				if (bA > PI_1)
					inShadow = true;
				else
					inShadow = false;
			}

			void Path::ValidPath()
			{
				valid = true;
				if ((zA < 0) || (zA > wData.z))	// Config control over allow virtual zA?
				{
					zValid = false;
					valid = false;
				}
				else
					zValid = true;

				if ((sData.t < 0) || (sData.t > wData.t))
				{
					sValid = false;
					valid = false;
				}
				else
					sValid = true;

				if ((rData.t < 0) || (rData.t > wData.t))
				{
					rValid = false;
					valid = false;
				}
				else
					rValid = true;
			}

			void Path::CalcR()
			{
				CalcR(&sData);
				CalcR(&rData);
			}

			void Path::CalcR(SRData* data)
			{
				data->r = (Cross(mEdge.GetAP(data->point), mEdge.mEdgeVector)).Length();
			}

			void Path::CalcZ()
			{
				CalcZ(&sData);
				CalcZ(&rData);
			}

			void Path::CalcZ(SRData* data)
			{
				vec3 AP = mEdge.GetAP(data->point);
				data->z = AP.Length() * Dot(UnitVector(AP), mEdge.mEdgeVector);
			}

			void Path::CalcT()
			{
				CalcT(&sData);
				CalcT(&rData);

				CorrectT();
			}

			void Path::CorrectT()
			{
				Real halfThetaW = wData.t / 2.0;
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

			void Path::CalcT(SRData* data)
			{
				vec3 k = UnitVector(data->point - mEdge.GetEdgeCoord(data->z));
				vec3 edgeNorm = mEdge.mEdgeNormal;
				data->t = acos(Dot(k, edgeNorm));
				data->rot = signbit(Dot(Cross(k, edgeNorm), mEdge.mEdgeVector));
			}

			void Path::CalcApex()
			{
				Real dZ = fabs(rData.z - sData.z) * sData.r / (sData.r + rData.r);
				if (sData.z > rData.z)
					zA = sData.z - dZ;
				else
					zA = sData.z + dZ;
				phi = atan(sData.r / dZ);
			}

			void Path::CalcD()
			{
				CalcD(&sData);
				CalcD(&rData);
			}

			void Path::CalcD(SRData* data)
			{
				vec3 apex = mEdge.GetEdgeCoord(zA);
				data->d = (data->point - mEdge.GetEdgeCoord(zA)).Length();
			}
		}
	}
}