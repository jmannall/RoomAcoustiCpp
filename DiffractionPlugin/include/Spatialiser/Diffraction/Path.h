/*
*
*  \Diffraction path class
*
*/

#ifndef Spatialiser_Diffraction_Path_h
#define Spatialiser_Diffraction_Path_h

#include "Common/vec3.h"
#include "Spatialiser/Edge.h"

namespace UIE
{
	using namespace Common;
	namespace Spatialiser
	{
		namespace Diffraction
		{

			//////////////////// Data structs ////////////////////

			struct SRData
			{
				vec3 point;
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
				Path(const vec3& source, const vec3& receiver, const Edge& edge);
				~Path() {};

				// Updates
				void UpdateParameters(const vec3& source, const vec3& receiver, const Edge& edge);
				void UpdateParameters(const vec3& source, const vec3& receiver);
				void UpdateParameters(const vec3& receiver);

				// Getters
				inline Real GetD(Real z) const { return sqrt(pow(sData.r, 2.0) + pow(z - sData.z, 2.0)) + sqrt(pow(rData.r, 2.0) + pow(z - rData.z, 2.0)); }
				inline Real GetMaxD() const { return std::max(GetD(0), GetD(wData.z)); }
				inline vec3 GetApex() const
				{
					if (zA < 0)
						return mEdge.GetEdgeCoord(0);
					else if (zA > wData.z)
						return mEdge.GetEdgeCoord(wData.z);
					else
						return mEdge.GetEdgeCoord(zA); // precalculate at update?
				}

				// Edge
				inline Edge GetEdge() const { return mEdge; }
				inline vec3 CalculateVirtualPostion() const { return rData.point + (sData.d + rData.d) * (mEdge.GetEdgeCoord(zA) - rData.point) / rData.d; }
				inline vec3 CalculateVirtualRPostion() const { return sData.point + (sData.d + rData.d) * (mEdge.GetEdgeCoord(zA) - sData.point) / sData.d; }

				// Variables
				SRData sData;
				SRData rData;
				WData wData;
				Real bA;
				Real mA;
				Real zA;
				Real phi;

				// Booleans
				bool zValid;
				bool sValid;
				bool rValid;
				bool valid;
				bool inShadow;

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
		}
	}
}

#endif