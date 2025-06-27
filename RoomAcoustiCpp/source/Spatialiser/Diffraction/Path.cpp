/*
*
*  \Diffraction path class
*
*/

// Spatialiser headers
#include "Spatialiser/Diffraction/Path.h"

namespace RAC
{
	namespace Spatialiser
	{
		namespace Diffraction
		{
			//////////////////// Path Class ////////////////////

			////////////////////////////////////////

			void Path::CalculateParameters()
			{
				eData.t = mEdge.GetExteriorAngle();
				eData.z = mEdge.GetLength();
				CalculateR();
				CalculateZ();
				CalculateT();
				CalculateApex();
				CalculateD();	// Uses zA
				CalculateBaMa();
				ValidPath();
			}

			////////////////////////////////////////

			void Path::CalculateT(SRData* data)
			{
				Vec3 k = UnitVector(data->point - mEdge.GetEdgeCoord(data->z));
				data->t = acos(Dot(k, mEdge.GetEdgeNormal()));
				data->rot = signbit(Dot(Cross(k, mEdge.GetEdgeNormal()), mEdge.GetEdgeVector()));
			}

			////////////////////////////////////////

			void Path::CorrectT()
			{
				Real halfThetaW = eData.t / 2.0;
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

			////////////////////////////////////////

			void Path::CalculateApex()
			{
				Real dZ = abs(rData.z - sData.z) * sData.r / (sData.r + rData.r);
				zA = sData.z > rData.z ? sData.z - dZ : sData.z + dZ;
				phi = atan(sData.r / dZ);
			}

			////////////////////////////////////////

			void Path::CalculateBaMa()
			{
				bA = abs(rData.t - sData.t);
				mA = std::min(sData.t, eData.t - rData.t);
				inShadowZone = bA > PI_1;
				inRelfZone = 2 * mA + bA <= PI_1;
			}

			////////////////////////////////////////

			void Path::ValidPath()
			{
				valid = true;
				if ((zA < 0) || (zA > eData.z))	// Config control over allow virtual zA?
				{
					zValid = false;
					valid = false;
				}
				else
					zValid = true;

				if ((sData.t < 0) || (sData.t > eData.t))
				{
					sValid = false;
					valid = false;
				}
				else
					sValid = true;

				if ((rData.t < 0) || (rData.t > eData.t))
				{
					rValid = false;
					valid = false;
				}
				else
					rValid = true;
			}
		}
	}
}