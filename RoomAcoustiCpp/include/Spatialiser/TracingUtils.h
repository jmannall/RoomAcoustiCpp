/*
* @brief Defines utilities for basic ray-tracing
*/

#ifndef Tracing_Utils_h
#define Tracing_Utils_h

#include "Common/Definitions.h" // Required for epsilons
#include "Spatialiser/TracingTypes.h"

namespace RAC
{
    using namespace Common;
	namespace Spatialiser
	{
        // TODO: Consider making one abstract class Ray which gets inherited by RayBundle, RayPencil, RaySingle.

        /**
         * @brief Intersect one triangle against one ray.
         * Uses different algorithms depending on #ifdef PLUCKER_KERNEL and #ifdef LEAN_PLUCKER:
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
         * Uses different algorithms depending on #ifdef PLUCKER_KERNEL and #ifdef LEAN_PLUCKER:
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
         * @param triangleIdxFront Pointer receiving the triangle index of nearest t > 0 hit, or -1 for invalid hits.
         * @param distanceFront Pointer receiving the line parameter |t| of nearest t > 0 hit, or NaN for invalid hits.
         * @param cosineFront Pointer receiving the incidence cosine of nearest t > 0 hit, or NaN for invalid hits.
         * @param triangleIdxBack Pointer receiving the triangle index of nearest t < 0 hit, or -1 for invalid hits.
         * @param distanceBack Pointer receiving the line parameter |t| of nearest t < 0 hit, or NaN for invalid hits.
         * @param cosineBack Pointer receiving the incidence cosine of nearest t < 0 hit, or NaN for invalid hits.
         * @param ignoredTriangleIndex Index of the triangle to be ignored (self-hit avoidance).
         */
        void trace_ray(
            const TriangleMeshSoA& triangles, const RayBundleSoA& rays, int rayIndex,
            int& triangleIdxFront, Real& distanceFront, Real& cosineFront,
            int& triangleIdxBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex = -1);
        // Overloaded for ray pencil
        void trace_ray(
            const TriangleMeshSoA& triangles, const RayPencilSoA& rays, int rayIndex,
            int& triangleIdxFront, Real& distanceFront, Real& cosineFront,
            int& triangleIdxBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex = -1);
        // Overloaded for single ray
        void trace_ray(
            const TriangleMeshSoA& triangles, const Vec3& rayOrigin, const Vec3& rayDirection,
            int& triangleIdxFront, Real& distanceFront, Real& cosineFront,
            int& triangleIdxBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex = -1);

        /* @brief Class for tracing a bundle of rays with different origins and directions. */
        class RayBundle {
        private:
            int numRays;
            RayBundleSoA rays;
            
            Vec<Real> radiance, totalDistance, latestDistance, latestCosine;
            std::vector<int> latestTriangleIdx, previousTriangleIdx;

        public:

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

            Vec<Real> frontDistance, backDistance, frontCosine, backCosine;
            std::vector<int> frontTriangleIdx, backTriangleIdx;

        public:
            /* @brief Initialization given one origin point.
              * @param origin 3D coordinates of the origin point.
              * @param numDirections Number of directions to sample uniformly on the unit sphere.
              * @param hemisphereOnly If true, sample numDirections on the unit hemisphere (positive Z only) instead of the unit sphere.
              */
            RayPencil(int numDirections, bool hemisphereOnly);

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

            /* @brief Cluster this pencil's directions based on their cosine similarity to a given set of directions. N.B.: assumes all directions are normalized.
             *
             * This function returns (by reference) two arrays of integer values, `frontClusters` and `backClusters`.
             * Both arrays have size `numRays` and contain values in the range [-1, directions.size()).
             * For each ray direction in RayPencil, this function computes the cosine similarity between that ray direction and all reference directions,
             *  as well as the cosine similarity between the opposite ray direction and all reference directions.
             * If the smallest cosine similarity is found between the ray direction and a reference direction, then
             *      frontClusters[ray_idx] is set to the index of the reference direction, and
             *      backClusters[ray_idx] is set to -1.
             * If the smallest cosine similarity is found between the OPPOSITE of the ray direction and a reference direction, then
             *      frontClusters[ray_idx] is set to -1, and
             *      backClusters[ray_idx] is set to the index of the reference direction.
             * Note that `frontClusters[i] == -1` <==> `backClusters[i] != -1`.
             *
             * @param directions The set of directions used for clustering.
             * @param frontClusters Pointer to a pre-allocated integer buffer of size `numRays`, used for output values (see notes).
             * @param backClusters Pointer to a pre-allocated integer buffer of size `numRays`, used for output values (see notes).
             */
            void clusterDirections(const std::vector<Vec3>& directions, std::vector<int>& frontClusters, std::vector<int>& backClusters) const;

            /* @brief For each ray, returns (by reference) the direction.
              */
            void getDirections(std::vector<Vec3>& directions) const;

            /* @brief For each ray, returns (by reference) the distance in meters to the nearest front and back intersections.
              * Values of NaN denote invalid intersections.
              */
            void getDistances(Vec<>& front, Vec<>& back) const;

            /* @brief For each ray, returns (by reference) the incidence cosine of the nearest front and back intersections.
              * Values of NaN denote invalid intersections.
              */
            void getCosines(Vec<>& front, Vec<>& back) const;

            /* @brief For each ray, returns (by reference) the triangle index of the nearest front and back intersections.
              * Values of invalidIdx denote invalid intersections.
              */
            void getIndices(std::vector<int>& front, std::vector<int>& back) const;
        };
	}
}

#endif