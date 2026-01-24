/*
* @class
*
* @brief Functions to help with edge locating
*
*/

#ifndef RoomAcoustiCpp_EdgeLocating_h
#define RoomAcoustiCpp_EdgeLocating_h

#include "Common/Definitions.h"
#include "Common/Vec3.h"

#include "Spatialiser/Edge.h"

namespace RAC
{
	using namespace Common;
	namespace Spatialiser
	{
		inline Real PointLinePosition(const Vec3& point, const Vec3& lineDir, const Vec3& linePoint)
		{
			Real t = (point - linePoint).dot(lineDir);

			if (IsApprox(t, REAL_CONST(0.0)))
				return REAL_CONST(0.0);

			Vec3 lineProj = linePoint + lineDir * t;
			return (point - lineProj).Normal();
		}

		inline int LocateEdgeOnLine(const Vertices& vertices, const Vec3& lineDir, const Vec3& linePoint)
		{
			// For a valid edge segment, two vertices must lie on line, and one vertex must lie to negative side of line
			bool foundNegative = false;
			int result = 0;

			Real k = PointLinePosition(vertices[0], lineDir, linePoint);
			if (IsApprox(k, REAL_CONST(0.0)))
				result++;
			else if (k < REAL_CONST(0.0))
				foundNegative = true;

			k = PointLinePosition(vertices[1], lineDir, linePoint);
			if (IsApprox(k, REAL_CONST(0.0)))
				result += 2;
			else if (k < REAL_CONST(0.0))
				foundNegative = true;

			if (result < 1) // No vertices found on line, no edge possible
				return 0;

			k = PointLinePosition(vertices[2], lineDir, linePoint);
			if (IsApprox(k, REAL_CONST(0.0)))
			{
				if (result == 3)
					return 0; // More than 2 vertices found on line, no edge possible
				result += 3;
			}
			else if (k < REAL_CONST(0.0))
				foundNegative = true;

			if (result < 3)	// No edge found
				return 0;

			return foundNegative ? -result : result;
		}

		/**
		* @brief Find the vertices that form an edge and lie on the given plane
		*
		* @return An integer code representing the located edge:
		* 0 - No valid edge found
		* 3 - Valid edge between vertex 0 and 1
		* 4 - Valid edge between vertex 0 and 2
		* 5 - Valid edge between vertex 1 and 2
		* if negative, the third vertex lies behind the plane
		*/
		inline int LocateEdgeOnPlane(const Vertices& vertices, const Plane& plane)
		{
			// For a valid edge segment, two vertices must lie on planeB, and one vertex must lie behind planeB
			bool foundNegative = false;
			int result = 0;

			Real k = plane.PointPlanePosition(vertices[0]);
			if (IsApprox(k, REAL_CONST(0.0)))
				result++;
			else if (k < REAL_CONST(0.0))
				foundNegative = true;

			k = plane.PointPlanePosition(vertices[1]);
			if (IsApprox(k, REAL_CONST(0.0)))
				result += 2;
			else if (k < REAL_CONST(0.0))
				foundNegative = true;
			 
			if (result < 1) // No vertices found on plane, no edge possible
				return 0;

			k = plane.PointPlanePosition(vertices[2]);
			if (IsApprox(k, REAL_CONST(0.0)))
			{
				if (result == 3)
					return 0; // More than 2 vertices found on line, no edge possible
				result += 3;
			}
			else if (k < REAL_CONST(0.0))
				foundNegative = true;

			if (result < 3)	// No edge found
				return 0;

			return foundNegative ? -result : result;
		}

		inline std::vector<EdgeSegment> MergeSegments(std::vector<EdgeSegment>& locatedSegments)
		{
			RAC_DEBUG_ASSERT(locatedSegments.size() >= 2, "MergeSegments called with less than 2 segments");

			// Sort segments by base value
			std::sort(locatedSegments.begin(), locatedSegments.end(),
				[](const EdgeSegment& a, const EdgeSegment& b) { return a.base < b.base; });

			std::vector<EdgeSegment> segments;

			// Merge overlapping segments
			int numSegments = ToInt(locatedSegments.size());
			EdgeSegment& current = locatedSegments[0];
			for (int i = 1; i < numSegments; i++)
			{
				EdgeSegment& next = locatedSegments[i];
				if (current.base >= current.top)	// Invalid current segment
				{
					current = next;
					continue;
				}

				if (IsApprox(current.top, next.base) && current.valid == next.valid) // Touching segments with same validity
				{
					current.top = next.top;
					continue;	// Merge two segments
				}

				if (current.top < next.base) // No overlap
				{
					segments.push_back(current);
					current = next;
					continue;	// Commit current segment and advance
				}

				// Commit current segment up to start of next segment
				segments.emplace_back(current.base, next.base, current.valid);

				if (next.top >= current.top) // Next extends beyond current
				{
					segments.emplace_back(next.base, current.top, false); // Commit invalid overlapping segment and advance
					next.base = current.top; // New current segment starts at end of old current
					current = next;
				}
				else // Next lies completely inside current
					segments.emplace_back(next.base, next.top, false); // Commit invalid overlapping segment
			}
			if (segments.size() < 1 || segments.back().top <= current.base) // All segments were merged or no overlap on last loop run
				segments.push_back(current);
			return segments;
		}

		inline std::vector<std::pair<Real, Real>> FindSegmentIntersections(const std::vector<EdgeSegment>& segmentsA, const std::vector<EdgeSegment>& segmentsB)
		{
			std::vector<std::pair<Real, Real>> result;
			int i = 0, j = 0;
			int numA = ToInt(segmentsA.size());
			int numB = ToInt(segmentsB.size());
			while (i < numA && j < numB)
			{
				const EdgeSegment& segmentA = segmentsA[i];
				const EdgeSegment& segmentB = segmentsB[j];

				if (segmentA.valid && segmentB.valid) // Both segments valid
				{
					if (segmentA.top > segmentB.base && segmentB.top > segmentA.base) // There is an overlap
					{
						// Compute overlap interval
						Real base = std::max(segmentA.base, segmentB.base);
						Real top = std::min(segmentA.top, segmentB.top);
						result.emplace_back(base, top);
					}
				}

				// Advance the index whose segment ends first
				if (IsApprox(segmentA.top, segmentB.top))
				{ i++; j++; }
				else if (segmentA.top < segmentB.top)
					i++;
				else
					j++;
			}
			return result;
		}

		/**
		* @brief Reconstructs a point on the line of intersection of two planes
		* 
		* @details Solve for x and y: offset = x normalA + y normalB
		* must satisfy: normalA.dot(offset) = D_A, normalB.dot(offset) = D_B
		* Give the following system of equations:
		* x normalA.dot(normalA) + y normalA.dot(normalB) = D_A
		* x normalB.dot(normalA) + y normalB.dot(normalB) = D_B
		* solve using Cramer's rule: https://www.cuemath.com/algebra/cramers-rule/
		* 
		* @param planeA The first plane
		* @param planeB The second plane
		* @param lineDir The direction of the line of intersection
		* @param k The distance along the line direction from the reconstructed point
		* @return The reconstructed point
		*/
		Vec3 ReconstructPoint(const Plane& planeA, const Plane& planeB, const Vec3& lineDir, Real k)
		{
			Vec3 normalA = planeA.GetNormal();
			Vec3 normalB = planeB.GetNormal();
			Real DA = planeA.GetD();
			Real DB = planeB.GetD();

			Real A = normalA.dot(normalA);
			Real B = normalA.dot(normalB);
			Real C = normalB.dot(normalB);

			// Only valid for non-parallel planes
			Real det = A * C - B * B;
			if (IsApprox(det, REAL_CONST(0.0))) // TODO: Remove, temporary for trying parallel planes
				return lineDir * k;
			RAC_DEBUG_ASSERT(det != 0.0, "Determinant is zero");

			Real x = (DA * C - B * DB) / det;
			Real y = (DB * A - B * DA) / det;

			Vec3 offset = normalA * x + normalB * y;
			return lineDir * k + offset;
		}
	}	
}


#endif // RoomAcoustiCpp_EdgeLocating_h