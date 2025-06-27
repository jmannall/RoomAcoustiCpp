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
				Vec3 point{ 0.0, 0.0, 0.0 };
				Real r{ 0.0 }, z{ 0.0 }, t{ 0.0 }, d{ 0.0 };
				bool rot{ true };
				SRData() {}
			};

			struct EdgeData
			{
				Real z{ 0.0 }, t{ 0.0 };
				EdgeData() {}
			};

			//////////////////// Path class ////////////////////

			class Path // Currently only supports up to 1st order diffraction
			{
			public:
				/**
				* @breif Default constructor
				*/
				Path() {};

				/**
				* @brief Constructor that initailises a diffarction path
				* 
				* @param source The source position
				* @param receiver The receiver position
				* @param edge The edge that the path diffracts via
				*/
				Path(const Vec3& source, const Vec3& receiver, const Edge& edge) : mEdge(edge)
				{
					UpdateParameters(source, receiver);
				}

				/**
				* @brief Default deconstructor
				*/
				~Path() {};

				/**
				* @brief Update the path parameters
				* 
				* @param source The source position
				* @param receiver The receiver position
				* @param edge The edge that the path diffracts via
				*/
				inline void UpdateParameters(const Vec3& source, const Vec3& receiver, const Edge& edge)
				{
					mEdge = edge;
					UpdateParameters(source, receiver);
				}
				
				/**
				* @brief Update the path parameters
				*
				* @param source The source position
				* @param receiver The receiver position
				*/
				inline void UpdateParameters(const Vec3& source, const Vec3& receiver)
				{
					sData.point = source;
					rData.point = receiver;
					CalculateParameters();
				}

				/**
				* @brief Reflect the stored edge in a plane
				* 
				* @param plane The plane the reflect the edge in
				*/
				void ReflectEdgeInPlane(const Plane& plane) { mEdge.ReflectInPlane(plane); }

				/**
				* @brief Returns the distance between the source and receiver for a given z coordinate
				* 
				* @param z The z coordinate to calculate the distance for
				* @return The distance between the source and receiver
				*/
				inline Real GetD(Real z) const
				{ 
					Real sZ = z - sData.z;
					Real rZ = z - rData.z;
					return sqrt(sData.r * sData.r + sZ * sZ) + sqrt(rData.r * rData.r + rZ * rZ);
				}

				/**
				* @return The maximum distance between the source and receiver via the edge
				*/
				inline Real GetMaxD() const { return std::max(GetD(0), GetD(eData.z)); }

				/**
				* @return The minimum distance between the source and receiver via the edge
				* 
				* @remarks Could precalculate at update?
				*/
				inline Vec3 GetApex() const { return mEdge.GetEdgeCoord(GetApexZ()); }

				/**
				* @return The z coordinate of the apex point
				*/
				inline Real GetApexZ() const
				{
					if (zA < 0)
						return 0;
					else if (zA > eData.z)
						return eData.z;
					else
						return zA;
				}

				/**
				* @return The edge that the path diffracts via
				*/
				const inline Edge& GetEdge() const { return mEdge; }

				/**
				* @return The virtual position of the source. This projects the source from the receiver via the apex over the distance dR + dS.
				*/
				inline Vec3 CalculateVirtualPostion() const { return rData.point + (sData.d + rData.d) * (GetApex() - rData.point) / rData.d; }

				bool zValid{ false };								// True is the apex point is on the physical edge, false otherwise
				bool sValid{ false }, rValid{ false };				// True if the source and receiver are in exterioir region of the wedge, false otherwise
				bool valid{ false };								// True if the path is valid, false otherwise
				bool inRelfZone{ false }, inShadowZone{ false };	// True if the receiver is in the reflection zone or shadow zone, false otherwise

				SRData sData, rData;			// Source and receiver data
				EdgeData eData;					// Edge data
				Real mA{ 0.0 }, bA{ 0.0 };		// Minimum and bending angle
				Real zA{ 0.0 }, phi{ 0.0 };		// Apex z coordinate and phi angle

			private:

				/**
				* @
				*/
				void CalculateParameters();

				/**
				* @brief Update the source and receiver r values
				*/
				inline void CalculateR() { CalculateR(&sData); CalculateR(&rData); }

				/**
				* @brief Update the source and receiver z values
				*/
				inline void CalculateZ() { CalculateZ(&sData); CalculateZ(&rData); }

				/**
				* @brief Update the r value for the given SRData
				*/
				inline void CalculateR(SRData* data) { data->r = (Cross(mEdge.GetAP(data->point), mEdge.GetEdgeVector())).Length(); }
				
				/**
				* @brief Update the z value for the given SRData
				*/
				inline void CalculateZ(SRData* data)
				{
					Vec3 AP = mEdge.GetAP(data->point);
					data->z = AP.Length() * Dot(UnitVector(AP), mEdge.GetEdgeVector());
				}

				/**
				* @brief Update the source and receiver theta values
				*/
				inline void CalculateT()
				{
					CalculateT(&sData);
					CalculateT(&rData);
					CorrectT();
				}

				/**
				* @brief Update the theta value for the given SRData
				*/
				void CalculateT(SRData* data);

				/**
				* @brief Correct the rotation of the source and receiver t values
				*/
				void CorrectT();

				/**
				* @brief Update the source and receiver d values
				*/
				inline void CalculateD() { CalculateD(&sData); CalculateD(&rData); }

				/**
				* @brief Update the d value for the given SRData
				*/
				inline void CalculateD(SRData* data) { data->d = (data->point - GetApex()).Length(); }

				/**
				* @brief Calculate the zA value and phi angle
				*/
				void CalculateApex();

				/**
				* @brief Calculate the bending and minimum angle. Determine if it is in the shadow or reflection zone.
				*/
				void CalculateBaMa();

				/**
				* @brief Check if the path is valid
				*/
				void ValidPath();

				Edge mEdge;		// Edge the path diffracts via
			};

			/**
			* @return True if paths are identical, false otherwise
			*/
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

				if (u.eData.t != v.eData.t)
					return false;
				if (u.eData.z != v.eData.z)
					return false;
				
				return true;
			}
		}
	}
}

#endif