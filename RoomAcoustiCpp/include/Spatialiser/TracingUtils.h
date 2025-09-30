/*
* @brief Defines utilities for basic ray-tracing
*/

#ifndef Tracing_Utils_h
#define Tracing_Utils_h

#include "Common/Definitions.h" // Required for epsilons
#include "Spatialiser/TracingTypes.h"

#if defined _DEBUG || defined DEBUG_RTM
#pragma optimize("", off)
#endif

namespace RAC
{
    using namespace Common;
	namespace Spatialiser
	{
        // TODO: Consider making one abstract class Ray which gets inherited by RayBundle, RayPencil, RaySingle.

        /**
         * @brief Intersect one triangle against one ray.
         * Uses different algorithms depending on #if PLUCKER_KERNEL and #if LEAN_PLUCKER:
         * lean Plücker, fat Plücker, or Möller–Trumbore.
         *
         * The kernel enforces:
         *  - the triangle's normal faces the ray's origin (dot(n,O)+d0 > eps_face)
         *  - the intersection point is inside the triangle (edges included)
         * It does not enforce t > 0 (line–triangle, not ray–triangle).
         *
         * @param triangles Structure of Arrays containing the triangles.
         * @param triangleIndex Index of the triangle to test.
         * @param rays Structure of Arrays containing the rays.
         * @param rayIndex Index of the ray to test.
         * @param distance Pointer to an output buffer for the line parameter t. Returns NaN for invalid hits.
         * @param cosine Pointer to an output buffer for the incidence cosine, i.e., dot(n,D). Returns NaN for invalid hits.
         */
        void intersection_test(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const RayBundleSoA& rays, int rayIndex,
            Real& distance, Real& cosine);
        // Overloaded for ray pencil
        void intersection_test(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const RayPencilSoA& rays, int rayIndex,
            Real& distance, Real& cosine);
        // Overloaded for single ray
        void intersection_test(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const Vec3& rayOrigin, const Vec3& rayDirection,
            Real& distance, Real& cosine);

        /**
         * @brief Intersect all triangles against one ray.
         * Uses different algorithms depending on #if PLUCKER_KERNEL and #if LEAN_PLUCKER:
         * lean Plücker, fat Plücker, or Möller–Trumbore.
         *
         * The kernel enforces:
         *  - the triangle's normal faces the ray's origin (dot(n,O)+d0 > eps_face)
         *  - the intersection point is inside the triangle (edges included)
         * It does not enforce t > 0 (line–triangle, not ray–triangle).
         * 
         * In the case of Z-fighting, the lower tirangle index wins.
         *
         * @param triangles Structure of Arrays containing the triangles.
         * @param rays Structure of Arrays containing the rays.
         * @param rayIndex Index of the ray to test.
         * @param patchIdFront Pointer receiving the node ID of nearest t > 0 hit, or -1 for invalid hits.
         * @param distanceFront Pointer receiving the line parameter |t| of nearest t > 0 hit, or NaN for invalid hits.
         * @param cosineFront Pointer receiving the incidence cosine of nearest t > 0 hit, or NaN for invalid hits.
         * @param patchIdBack Pointer receiving the node ID of nearest t < 0 hit, or -1 for invalid hits.
         * @param distanceBack Pointer receiving the line parameter |t| of nearest t < 0 hit, or NaN for invalid hits.
         * @param cosineBack Pointer receiving the incidence cosine of nearest t < 0 hit, or NaN for invalid hits.
         * @param ignoredTriangleIndex Index of the triangle to be ignored (self-hit avoidance).
         */
        void trace_ray(
            const TriangleMeshSoA& triangles, const RayBundleSoA& rays, int rayIndex,
            int& patchIdFront, Real& distanceFront, Real& cosineFront,
            int& patchIdBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex = -1);
        // Overloaded for ray pencil
        void trace_ray(
            const TriangleMeshSoA& triangles, const RayPencilSoA& rays, int rayIndex,
            int& patchIdFront, Real& distanceFront, Real& cosineFront,
            int& patchIdBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex = -1);
        // Overloaded for single ray
        void trace_ray(
            const TriangleMeshSoA& triangles, const Vec3& rayOrigin, const Vec3& rayDirection,
            int& patchIdFront, Real& distanceFront, Real& cosineFront,
            int& patchIdBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex = -1);

        /* @brief Class for tracing a bundle of rays with different origins and directions. */
        class RayBundle {
        private:
            int numRays;
            RayBundleSoA rays;
            
            Vec<Real> radiance, totalDistance, latestDistance, latestCosine;
            std::vector<int> latestPatchId, previousPatchId;

        public:
            /* @brief Default constructor (0 rays)
              */
            RayBundle();

            /* @brief Initialization given one origin point (same for all rays) and separate directions (different for every ray).
             */
            RayBundle(const Vec3& origin, const std::vector<Vec3>& directions);

            /* @brief Initialization given separate origin points and directions (different for every ray).
             */
            RayBundle(const std::vector<Vec3>& origins, const std::vector<Vec3>& directions);

            /* @brief Find the next intersection point of each ray and update the intersected triangle indices, without advancing the rays.
             */
            void traceAll(const TriangleMeshSoA& triangles);

            /* @brief Advance every ray to its next intersection point, updating all origin points and travel distances;
             *  update directions based on the previously intersected polygons' scattering coefficients,
             *  and radiance values based on the previously intersected polygons' absorption coefficients.
             *  */
            // TODO: Define this function if we ever want to trace multiple reflection orders.
            // TODO: Pass pointers to the absorption and scattering coeffs. to use for the reflections.
            void advanceAndReflect(const TriangleMeshSoA& triangles);

            /* @brief Returns the number of rays in the bundle.
              */
            inline int getNumRays() const { return numRays; }

            /* @brief For each ray, returns (by reference) the origin.
              */
            void getOrigins(std::vector<Vec3>& origins) const;

            /* @brief For each ray, returns (by reference) the direction.
              */
            void getDirections(std::vector<Vec3>& directions) const;

            /* @brief For each ray, returns the total travel distance in meters. Values of NaN denote invalid intersections.
             * The distance depends on which function was called most recently:
             *  if "advance()" was called more recently than "trace()":
             *      the distance refers to the intersection at "latest_intersected_triangle".
             *  if "trace()" was called more recently than "advance()":
             *      the distance refers to the intersection at "previous_intersected_triangle".
             */
            void getTotalDistances(Vec<>& distances) const;

            /* @brief For each ray, returns the incidence cosine of the latest intersection. Values of NaN denote invalid intersections.
             * The cosine depends on which function was called most recently:
             *  if "advance()" was called more recently than "trace()":
             *      the cosine refers to the intersection at "latest_intersected_triangle".
             *  if "trace()" was called more recently than "advance()":
             *      the cosine refers to the intersection at "previous_intersected_triangle".
             */
            void getCosines(Vec<>& cosines) const;

            /* Returns the indices of the previous and current polygon intersected by each ray.
             * Values of invalidIdx denote invalid intersections.
             */
            void getIndices(std::vector<int>& current, std::vector<int>& previous) const;

            /* Returns every ray's radiance value. Values of NaN denote invalid intersections.
             */
            void getRadiance(Vec<>& radiance) const;
        };

        /* @brief Class for tracing a pencil of rays with shared origin and different directions. */
        class RayPencil {
        private:
            int numRays;
            RayPencilSoA rays;

            // If this is true, `clusterDirections()` and all `get_()` methods behave as if this instance contained
            //  twice as many rays as it actually does. They will report the results related to the real "forward" rays,
            //  and then concatenate the results for their direct opposite directions. Tracing is performed only once.
            bool exposeMirrorCopies = false;

            Vec<Real> frontDistance, backDistance, frontCosine, backCosine;
            std::vector<int> frontpatchId, backpatchId;

        public:
            /* @brief Default constructor (0 rays)
              */
            RayPencil();

            /* @brief Initialization given one origin point.
              * @param origin 3D coordinates of the origin point.
              * @param numDirections Number of directions to sample uniformly on the unit sphere.
              * @param hemisphereOnly If true, sample numDirections on the unit hemisphere (positive Z only) instead of the unit sphere.
              */
            RayPencil(int numDirections, bool hemisphereOnly = true);

            /* @brief Initialization given one origin point (same for all rays) and separate directions (different for every ray).
              */
            RayPencil(const std::vector<Vec3>& directions);

            /* @brief Update the origin position, without changing the relative directions.
              * N.B.: This resets all intersection data.
              */
            void moveOrigin(const Vec3& origin);

            /* @brief Find the next intersection point of each ray, in the front and back.
              */
            void traceAll(const TriangleMeshSoA& triangles);

            /* @brief Cluster this pencil's directions based on their cosine similarity to a given set of directions. N.B.: assumes input directions are normalized.
             *
             * This function returns (by reference) an array of integer values, `clusters`.
             * The array has size `numRays` and contains values in the range [-1, directions.size()).
             * For each ray in RayPencil, this function computes the cosine similarity (dot product) between that ray's direction and all reference directions.
             * The value of `clusters[ray_idx]` is the index of the reference direction with the highest cosine similarity to ray `ray_idx`.
             * If `directions` is empty, all output values are set to -1.
             *
             * @param directions The set of directions used for clustering. If these have non-unit norms, it will result in "weighted" clusters.
             * @param clusters Pointer to a pre-allocated integer buffer of size `numRays`, used for output values.
             */
            void clusterDirections(const std::vector<Vec3>& directions, std::vector<int>& clusters) const;

            /* @brief Returns the number of rays in the pencil.
              */
            inline int getNumRays() const { return exposeMirrorCopies ? 2 * numRays : numRays; }

            /* @brief For each ray, returns (by reference) the direction.
              */
            void getDirections(std::vector<Vec3>& directions) const;

            /* @brief For each ray, returns (by reference) the distance in meters to the nearest front and back intersections.
              * Values of NaN denote invalid intersections.
              */
            void getDistances(Vec<>& distances) const;

            /* @brief For each ray, returns (by reference) the incidence cosine of the nearest front and back intersections.
              * Values of NaN denote invalid intersections.
              */
            void getCosines(Vec<>& cosines) const;

            /* @brief For each ray, returns (by reference) the node ID of the nearest front and back intersections.
              * Values of invalidIdx denote invalid intersections.
              */
            void getIndices(std::vector<int>& front, std::vector<int>& back) const;
        };
	}
}

#if defined _DEBUG || defined DEBUG_RTM
#pragma optimize("", on)
#endif

#endif