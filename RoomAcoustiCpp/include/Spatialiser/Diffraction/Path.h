/*
*
*  \Diffraction path class
*
*/

#ifndef RoomAcoustiCpp_Diffraction_Path_h
#define RoomAcoustiCpp_Diffraction_Path_h

#include "Common/Vec3.h"
#include "Spatialiser/Edge.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		namespace Diffraction
		{

			//////////////////// Data structs ////////////////////

			struct SRData
			{
				Vec3 point;
				Real r;
				Real z;
				Real t;
				Real d;
				bool rot;
				SRData() : r(0.0), z(0.0), t(0.0), d(0.0), rot(true) {}
			};

			struct WData
			{
				Real z;
				Real t;
				WData() : z(0.0), t(0.0) {}
			};

			//////////////////// Path class ////////////////////

			class Path // Currently only supports up to 1st order diffraction
			{
			public:
				// Load and Destroy
				Path() : zValid(false), sValid(false), rValid(false), valid(false), inShadow(false), bA(0.0), mA(0.0), zA(0.0), phi(0.0) {};
				Path(const Vec3& source, const Vec3& receiver, const Edge& edge);
				~Path() {};

				// Updates
				void UpdateParameters(const Vec3& source, const Vec3& receiver, const Edge& edge);
				void UpdateParameters(const Vec3& source, const Vec3& receiver);
				void UpdateParameters(const Vec3& receiver);
				void ReflectEdgeInPlane(const Plane& plane) { mEdge.ReflectInPlane(plane); }

				// Getters
				inline Real GetD(Real z) const
				{ 
					Real sZ = z - sData.z;
					Real rZ = z - rData.z;
					return sqrt(sData.r * sData.r + sZ * sZ) + sqrt(rData.r * rData.r + rZ * rZ);
				}
				inline Real GetMaxD() const { return std::max(GetD(0), GetD(wData.z)); }
				inline Vec3 GetApex() const
				{
					if (zA < 0)
						return mEdge.GetEdgeCoord(0);
					else if (zA > wData.z)
						return mEdge.GetEdgeCoord(wData.z);
					else
						return mEdge.GetEdgeCoord(zA); // precalculate at update?
				}

				inline Real GetApexZ() const
				{
					if (zA < 0)
						return 0;
					else if (zA > wData.z)
						return wData.z;
					else
						return zA;
				}

				// Edge
				const inline Edge& GetEdge() const { return mEdge; }
				inline Vec3 CalculateVirtualPostion() const { return rData.point + (sData.d + rData.d) * (mEdge.GetEdgeCoord(zA) - rData.point) / rData.d; }
				inline Vec3 CalculateVirtualRPostion() const { return sData.point + (sData.d + rData.d) * (mEdge.GetEdgeCoord(zA) - sData.point) / sData.d; }

				// Booleans
				bool zValid;
				bool sValid;
				bool rValid;
				bool valid;
				bool inShadow;

				// Variables
				SRData sData;
				SRData rData;
				WData wData;
				Real bA;
				Real mA;
				Real zA;
				Real phi;

			private:

				// Updates
				void UpdateParameters();
				void UpdateWData();
				void UpdateBaMa();

				// Caluculations
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

				// Member variables
				Edge mEdge;
			};

			inline bool operator==(const Path& u, const Path& v)
			{
				if (u.valid != v.valid)
					return false;

				if (u.sData.point != v.sData.point)
					return false;
				if (u.rData.point != v.rData.point)
					return false;
				
				if (u.bA != v.bA)
					return false;
				if (u.mA != v.mA)
					return false;
				if (u.zA != v.zA)
					return false;
				if (u.phi != v.phi)
					return false;

				if (u.wData.t != v.wData.t)
					return false;
				if (u.wData.z != v.wData.z)
					return false;
				
				return true;
			}
		}
	}
}

#endif