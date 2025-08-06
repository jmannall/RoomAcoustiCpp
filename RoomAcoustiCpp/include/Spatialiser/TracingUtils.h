/*
* @brief Defines utilities for basic ray-tracing
*/

#ifndef Tracing_Utils_h
#define Tracing_Utils_h

#include "Spatialiser/TracingTypes.h"

namespace RAC
{
	namespace Common
	{
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
            TriangleMeshSoA& triangles, int triangleIndex,
            RayBundleSoA& rays, int rayIndex,
            Real& distance, Real& cosine);
        // Overloaded for ray pencil
        void intersection_test(
            TriangleMeshSoA& triangles, int triangleIndex,
            RayPencilSoA& rays, int rayIndex,
            Real& distance, Real& cosine);
        // Overloaded for single ray
        void intersection_test(
            TriangleMeshSoA& triangles, int triangleIndex,
            Vec3& rayOrigin, Vec3& rayDirection, Vec3& rayMoment,
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
         * @param triangles Structure of Arrays containing the triangles.
         * @param rays Structure of Arrays containing the rays.
         * @param rayIndex Index of the ray to test.
         * @param triangleIdxFront Pointer receiving the triangle index of nearest t > 0 hit, or -1 for invalid hits.
         * @param distanceFront Pointer receiving the line parameter t of nearest t > 0 hit, or NaN for invalid hits.
         * @param triangleIdxBack Pointer receiving the triangle index of nearest t < 0 hit, or -1 for invalid hits.
         * @param distanceBack Pointer receiving the line parameter |t| of nearest t < 0 hit, or NaN for invalid hits.
         * @param ignoredTriangleIndex Index of the triangle to be ignored (self-hit avoidance).
         */
        void trace_ray(
            TriangleMeshSoA& triangles, RayBundleSoA& rays, int rayIndex,
            int& triangleIdxFront, Real& distanceFront, Real& cosineFront,
            int& triangleIdxBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex = -1);
        // Overloaded for ray pencil
        void trace_ray(
            TriangleMeshSoA& triangles, RayPencilSoA& rays, int rayIndex,
            int& triangleIdxFront, Real& distanceFront, Real& cosineFront,
            int& triangleIdxBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex = -1);
        // Overloaded for single ray
        void trace_ray(
            TriangleMeshSoA& triangles, Vec3& rayOrigin, Vec3& rayDirection, Vec3& rayMoment,
            int& triangleIdxFront, Real& distanceFront, Real& cosineFront,
            int& triangleIdxBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex = -1);

        /* @brief Class for tracing a bundle of rays with different origins and directions. */
        class RayBundle {
        private:
            int numRays;
            RayBundleSoA rays;

            Vec<Real> radiance, totalDistance, latestDistance;
            std::vector<int> latestIntersectedTriangle, previousIntersectedTriangle;

        public:

            /* @brief Initialization given one origin point (same for all rays) and separate directions (different for every ray).
             */
            RayBundle(Vec3& origin, Matrix<>& directions);

            /* @brief Initialization given separate origin points and directions (different for every ray).
             */
            RayBundle(Matrix<>& origins, Matrix<>& directions);

            /* @brief Find the next intersection point of each ray and update the intersected triangle indices, without advancing the rays.
             */
            void trace_all();

            /* @brief Advance every ray to its next intersection point, updating all origin points and travel distances;
             *  update directions based on the previously intersected polygons' scattering coefficients,
             *  and radiance values based on the previously intersected polygons' absorption coefficients.
             *  */
            void advance();

            /* @brief For each ray, returns the total travel distance in meters. Values of NaN denote invalid intersections.
             * The distance depends on which function was called most recently:
             *  if "advance()" was called more recently than "trace()":
             *      the distance refers to the intersection at "latest_intersected_triangle".
             *  if "trace()" was called more recently than "advance()":
             *      the distance refers to the intersection at "previous_intersected_triangle".
             */
            void get_total_distances(Vec<>& totalDistances);

            /* Returns the indices of the previous and current polygon intersected by each ray.
             * Values of invalidIdx denote invalid intersections.
             */
            void get_indices(std::vector<int>& currentIndices, std::vector<int>& previousIndices);

            /* Returns every ray's radiance value. Values of NaN denote invalid intersections.
             */
            Vec<Real> get_radiance();
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
            RayPencil(Vec3& origin, int numDirections, bool hemisphereOnly);

            /* @brief Initialization given one origin point (same for all rays) and separate directions (different for every ray).
              */
            RayPencil(Vec3& origin, Matrix<>& directions);

            /* @brief Update the origin position, without changing the relative directions.
             */
            void move_origin(Vec3& origin);

            /* @brief Find the next intersection point of each ray, in the front and back.
             */
            void trace_all();

            /* @brief For each ray, returns (by reference) the distance in meters to the nearest front and back intersections.
             * Values of NaN denote invalid intersections.
             */
            void get_distances(Vec<>& frontDistances, Vec<>& backDistances);

            /* @brief For each ray, returns (by reference) the incidence cosine of the nearest front and back intersections.
             * Values of NaN denote invalid intersections.
             */
            void get_cosines(Vec<>& frontCosines, Vec<>& backCosines);

            /* @brief For each ray, returns (by reference) the triangle index of the nearest front and back intersections.
             * Values of invalidIdx denote invalid intersections.
             */
            void get_indices(std::vector<int>& frontIndices, std::vector<int>& backIndices);
        };
	}
}

#endif