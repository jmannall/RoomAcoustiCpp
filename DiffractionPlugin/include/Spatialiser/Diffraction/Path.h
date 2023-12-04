#pragma once

#include "vec3.h"
#include "Spatialiser/Edge.h"

namespace Spatialiser
{
	namespace Diffraction
	{
		struct SRData
		{
			vec3 point;
			float r;
			float z;
			float t;
			float d;
			bool rot;
			SRData() : r(0.0f), z(0.0f), t(0.0f), d(0.0f), rot(true) {}
		};

		struct WData
		{
			float z;
			float t;
			WData() : z(0.0f), t(0.0f) {}
		};


		class Path // Currently only supports up to 1st order diffraction
		{
		public:
			Path() : zValid(false), sValid(false), rValid(false), valid(false), inShadow(false), bA(0.0f), mA(0.0f), zA(0.0f), phi(0.0f) {};
			Path(const vec3& source, const vec3& receiver, const Edge& edge);
			~Path() {};

			void UpdateParameters(const vec3& source, const vec3& receiver, const Edge& edge);
			void UpdateParameters(const vec3& source, const vec3& receiver);
			void UpdateParameters(const vec3& receiver);
			inline float GetD(float z) const {	return sqrtf(powf(sData.r, 2) + powf(z - sData.z, 2)) + sqrtf(powf(rData.r, 2) + powf(z - rData.z, 2)); }
			inline float GetMaxD() const { return std::max(GetD(0), GetD(wData.z)); }
			inline vec3 GetApex() const
			{
				if (zA < 0)
					return mEdge.GetEdgeCoord(0);
				else if (zA > wData.z)
					return mEdge.GetEdgeCoord(wData.z);
				else
					return mEdge.GetEdgeCoord(zA); // precalculate at update?
			}
			inline Edge GetEdge() const { return mEdge; }
			inline vec3 CalculateVirtualPostion() const { return rData.point + (sData.d + rData.d) * (mEdge.GetEdgeCoord(zA) - rData.point) / rData.d; }
			inline vec3 CalculateVirtualRPostion() const { return sData.point + (sData.d + rData.d) * (mEdge.GetEdgeCoord(zA) - sData.point) / sData.d; }

			SRData sData;
			SRData rData;
			WData wData;
			float bA;
			float mA;
			float zA;
			float phi;

			bool zValid;
			bool sValid;
			bool rValid;
			bool valid;
			bool inShadow;

		private:
			Edge mEdge;
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
	}
}