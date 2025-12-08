
// C++ headers
#include <cassert>
#include <omp.h>
#include <chrono>

// Common headers
#include "Common/Debug.h"

// Spatialiser headers
#include "Spatialiser/TracingUtils.h"

// Enable OMP at the triangle level. Using up to 4 threads can result in an improvement, but it is nowhere near as much as USE_OMP_RAYTRACE_ALL
#define USE_OMP_RAYTRACE_SINGLE            ( 0 )

// Enable OMP at the traceAll() level. This has a significant improvement, even up to 8 or more threads. Note: it only optimizes one version of the traceAll() for now
// since the other has data dependencies.
#define USE_OMP_RAYTRACE_ALL               ( 1 )

// Dumps some stats about the trace function time
#define PROFILE_TRACE_ALL                  ( 0 )

namespace RAC
{
    using namespace Common;
    namespace Spatialiser
    {
        // ------------------------ Intersection kernels ------------------------

        bool intersection_test_internal(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const Vec3& O, const Vec3& D,
            Real& distance, Real& cosine)
        {
			// Users should test the return result, but default the results to qNaN just in case
			distance = std::numeric_limits<Real>::quiet_NaN();
			cosine = std::numeric_limits<Real>::quiet_NaN();

			// Sanity check: the requested triangle must exist.
            RAC_DEBUG_ASSERT(triangleIndex >= 0, "Triangle index out of bounds: " + ToString(triangleIndex));
            RAC_DEBUG_ASSERT(triangleIndex < ToInt(triangles.size()), "Triangle index out of bounds: " + ToString(triangleIndex));

			// Load plane data into locals.
			const Vec3 &n = triangles.n[triangleIndex];
			const Real d0WithEPS = triangles.d0PlusEPS[triangleIndex];

			// Facing test.
			if (n.dot(O) < d0WithEPS) {
				return false;
			}

			// Load "Möller–Trumbore" triangle data into locals.
			const Vec3& A = triangles.A[triangleIndex];
			const Vec3& e1 = triangles.edge1[triangleIndex];
			const Vec3& e2 = triangles.edge2[triangleIndex];

			// Möller–Trumbore barycentric numerators (unnormalized).
			const Vec3 pvec = D.cross(e2);
			const Real det = e1.dot(pvec);
			const Vec3 tvec = O - A;
			const Real u_num = tvec.dot(pvec);
			const Vec3 qvec = tvec.cross(e1);
			const Real v_num = D.dot(qvec);

			// Early-out on u and v having opposite signs (edges included via epsilon).
            // either value is close to EPS_EDGE, treat them as the same sign
            if ( abs(u_num) > EPS_EDGE && abs(v_num) > EPS_EDGE)
            {
                // otherwise make sure the signs are ok
                if (std::signbit(u_num) != std::signbit(v_num))
                    return false;
            }

			const Real w_num = det - (u_num + v_num);

            // We know that u_num & v_num have the same sign, so we just need to make sure that w_num is
            if (std::abs(w_num) > EPS_EDGE)
            {
                // check if the sign is equal to either u_num && v_num
                if (abs(u_num) > EPS_EDGE && std::signbit(u_num) != std::signbit(w_num))
                    return false;
				if (abs(v_num) > EPS_EDGE && std::signbit(v_num) != std::signbit(w_num))
					return false;
            }

			// Parallel test + compute t.
			if (std::abs(det) <= EPS_PARALLEL)
			{
				return false;
			}

        	const Real t_num = e2.dot(qvec); // e2 · qvec
			distance = t_num / det;
			cosine = std::abs(n.dot(D));
			return true;
        }


        bool intersection_test(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const RayBundleSoA& rays, int rayIndex,
            Real& distance, Real& cosine)
        {
            // Sanity check: the requested ray must exist.
            RAC_DEBUG_ASSERT(rayIndex >= 0, "Ray index out of bounds: " + ToString(rayIndex));
            RAC_DEBUG_ASSERT(rayIndex < ToInt(rays.size()), "Ray index out of bounds" + ToString(rayIndex));

            // Load ray data into locals.
            const Vec3& O = rays.O[rayIndex];
            const Vec3& D = rays.D[rayIndex];

            return intersection_test_internal(triangles, triangleIndex, O, D, distance, cosine);
        }

        bool intersection_test(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const RayPencilSoA& rays, int rayIndex,
            Real& distance, Real& cosine)
        {
            // Sanity check: the requested ray must exist.
            RAC_DEBUG_ASSERT(rayIndex >= 0, "Ray index out of bounds: " + ToString(rayIndex));
            RAC_DEBUG_ASSERT(rayIndex < ToInt(rays.size()), "Ray index out of bounds" + ToString(rayIndex));

            // Load ray data into locals.
            const Vec3& O = rays.O;
        	const Vec3& D = rays.D[rayIndex];

			return intersection_test_internal(triangles, triangleIndex, O, D, distance, cosine);
        }

		struct SingleRay
		{
			Vec3 rayOrigin;
			Vec3 rayDirection;
		};

        // This is a "fake" intersection_test that has the same signature as the others to reduce the code
		static bool intersection_test(
			const TriangleMeshSoA& triangles, int triangleIndex,
			const SingleRay &ray, int rayIndexNotUsed,
			Real& distance, Real& cosine)
		{
			// Load ray data into locals.
			const Vec3& O = ray.rayOrigin;
			const Vec3& D = ray.rayDirection;

			return intersection_test_internal(triangles, triangleIndex, O, D, distance, cosine);
		}

        bool intersection_test(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const Vec3& rayOrigin, const Vec3& rayDirection,
            Real& distance, Real& cosine)
        {
            // Load ray data into locals.
            const Vec3& O = rayOrigin;
            const Vec3& D = rayDirection;

			return intersection_test_internal(triangles, triangleIndex, O, D, distance, cosine);
        }

        // ------------------------ Tracing loop kernels ------------------------

        template <class TRayType>
        void trace_ray_internal(const TriangleMeshSoA& triangles, const TRayType& rays, int rayIndex,
            int& patchIdFront, Real& distanceFront, Real& cosineFront,
            int& patchIdBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex)
        {
			// Initialize per-ray front/back bests.
			patchIdFront = -1;
			patchIdBack = -1;
			distanceFront = std::numeric_limits<Real>::infinity();
			distanceBack = -std::numeric_limits<Real>::infinity();
			cosineFront = std::numeric_limits<Real>::quiet_NaN();
			cosineBack = std::numeric_limits<Real>::quiet_NaN();

#if USE_OMP_RAYTRACE_SINGLE
            constexpr int WorkerBlocks = 4;
            const int BlockSize = (triangles.size() + WorkerBlocks - 1) / WorkerBlocks;

            struct Instance {
                int patchIdFront = -1;
                int patchIdBack = -1;
                Real distanceFront = std::numeric_limits<Real>::infinity();
                Real distanceBack = -std::numeric_limits<Real>::infinity();
                Real cosineFront = std::numeric_limits<Real>::quiet_NaN();
                Real cosineBack = std::numeric_limits<Real>::quiet_NaN();
            };
            Instance instances[WorkerBlocks];

			#pragma omp parallel for num_threads(WorkerBlocks) shared(instances)
            for (int workerIndex = 0; workerIndex < WorkerBlocks; ++workerIndex) {
                Instance& instance = instances[workerIndex];

				const int start = workerIndex * BlockSize;
				const int end = std::min(start + BlockSize, triangles.size());

#define TRACE_INSTANCE(n)       instance.n

#else
			{

                constexpr int start = 00;
                const int end = triangles.size();
#define TRACE_INSTANCE(n)       n

#endif

                for (int i = start; i < end; ++i) {
                    if (i == ignoredTriangleIndex) // Ignore this triangle
                        continue;

                    // Buffers to retrieve individual check results.
                    Real currentDist, currentCos;

                    if (!intersection_test(triangles, i, rays, rayIndex, currentDist, currentCos))
                        continue;

                    if ((currentDist + EPS_ZFIGHT < TRACE_INSTANCE(distanceBack)) || (currentDist - EPS_ZFIGHT > TRACE_INSTANCE(distanceFront)))
                        continue; // Outside of current best range
                    if (std::abs(currentDist) < EPS_SELFHIT)
                        continue; // Too close to origin

                    // Valid hit
                    if (currentDist > 0.0) {
                        if (std::abs(currentDist - TRACE_INSTANCE(distanceFront)) < EPS_ZFIGHT) {
                            // Z-fighting, lower triangle index wins.
                            if (i < TRACE_INSTANCE(patchIdFront)) {
                                TRACE_INSTANCE(patchIdFront) = triangles.patchId[i];
                                TRACE_INSTANCE(distanceFront) = currentDist;
                                TRACE_INSTANCE(cosineFront) = currentCos;
                            } // else {keep the previous best}
                        }
                        else {
                            TRACE_INSTANCE(patchIdFront) = triangles.patchId[i];
                            TRACE_INSTANCE(distanceFront) = currentDist;
                            TRACE_INSTANCE(cosineFront) = currentCos;
                        }
                    }
                    else {
                        if (std::abs(currentDist - TRACE_INSTANCE(distanceBack)) < EPS_ZFIGHT) {
                            // Z-fighting, lower triangle index wins
                            if (i < TRACE_INSTANCE(patchIdFront)) {
                                TRACE_INSTANCE(patchIdBack) = triangles.patchId[i];
                                TRACE_INSTANCE(distanceBack) = currentDist;
                                TRACE_INSTANCE(cosineBack) = currentCos;
                            } // else {keep the previous best}
                        }
                        else {
                            TRACE_INSTANCE(patchIdBack) = triangles.patchId[i];
                            TRACE_INSTANCE(distanceBack) = currentDist;
                            TRACE_INSTANCE(cosineBack) = currentCos;
                        }
                    }
                }
			}

#if USE_OMP_RAYTRACE_SINGLE
			// Find the best candidate
		   for (int workerIndex = 0; workerIndex < WorkerBlocks; ++workerIndex)
            {
				const Instance& instance = instances[workerIndex];
                if (!std::isinf(instance.distanceFront)) 
                {
                    if (std::isinf(distanceFront) || distanceFront > instance.distanceFront)
                    {
                        distanceFront = instance.distanceFront;
                        cosineFront = instance.cosineFront;
                        patchIdFront = instance.patchIdFront;
                    }
                }
				if (!std::isinf(instance.distanceBack)) 
                {
                    const Real absDistance = -instance.distanceBack;
					if (std::isinf(distanceBack) || distanceBack > absDistance)
					{
						distanceBack = absDistance;
						cosineBack = instance.cosineBack;
						patchIdBack = instance.patchIdBack;
					}
				}

            }
#else
			if (std::isinf(distanceFront)) {
				// If it's still the initial INFINITY value, there was no valid hit at all.
				patchIdFront = -1;
				distanceFront = std::numeric_limits<Real>::quiet_NaN();
				cosineFront = std::numeric_limits<Real>::quiet_NaN();
			}
			if (std::isinf(distanceBack)) {
				// If it's still the initial -INFINITY value, there was no valid hit at all.
				patchIdBack = -1;
				distanceBack = std::numeric_limits<Real>::quiet_NaN();
				cosineBack = std::numeric_limits<Real>::quiet_NaN();
			}
			else // If the distanceBack is valid, return its absolute value.
				distanceBack = -distanceBack;
#endif
        }

        // Overloaded exposed signatures for cleanliness.
        void trace_ray(
            const TriangleMeshSoA& triangles, const RayBundleSoA& rays, int rayIndex,
            int& patchIdFront, Real& distanceFront, Real& cosineFront,
            int& patchIdBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex)
        {
            trace_ray_internal(triangles, rays, rayIndex, patchIdFront, distanceFront, cosineFront, patchIdBack, distanceBack, cosineBack, ignoredTriangleIndex);
        }

        void trace_ray(
            const TriangleMeshSoA& triangles, const RayPencilSoA& rays, int rayIndex,
            int& patchIdFront, Real& distanceFront, Real& cosineFront,
            int& patchIdBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex)
        {
			trace_ray_internal(triangles, rays, rayIndex, patchIdFront, distanceFront, cosineFront, patchIdBack, distanceBack, cosineBack, ignoredTriangleIndex);
        }

        void trace_ray(
            const TriangleMeshSoA& triangles, const Vec3& rayOrigin, const Vec3& rayDirection,
            int& patchIdFront, Real& distanceFront, Real& cosineFront,
            int& patchIdBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex)
        {
            trace_ray_internal(triangles, SingleRay{ rayOrigin, rayDirection }, 0, patchIdFront, distanceFront, cosineFront, patchIdBack, distanceBack, cosineBack, ignoredTriangleIndex);
        }

        // ------------------------ RayBundle methods ------------------------

        RayBundle::RayBundle()
        {
            numRays = 0;
            rays.resize(0);

            radiance = Vec<>::Constant(numRays, 1.0);
            totalDistance = Vec<>::Zero(numRays);
            latestDistance = Vec<>::Zero(numRays);
            latestCosine = Vec<>::Zero(numRays);
            latestPatchId = Vec<int>::Constant(numRays, -1);
            previousPatchId = Vec<int>::Constant(numRays, -1);
        }
        
        RayBundle::RayBundle(const Vec3& origin, const std::vector<Vec3>& directions)
        {
            numRays = ToInt(directions.size());
            rays.resize(numRays);

            for (int i = 0; i < numRays; ++i) {
                rays.O[i] = origin;
                rays.D[i] = directions[i];
            }
            rays.normalize_directions();

            radiance = Vec<>::Constant(numRays, 1.0);
            totalDistance = Vec<>::Zero(numRays);
            latestDistance = Vec<>::Zero(numRays);
            latestCosine = Vec<>::Zero(numRays);
            latestPatchId = Vec<int>::Constant(numRays, -1);
            previousPatchId = Vec<int>::Constant(numRays, -1);
        }

        RayBundle::RayBundle(const std::vector<Vec3>& origins, const std::vector<Vec3>& directions)
        {
            numRays = ToInt(directions.size());
            rays.resize(numRays);

            for (int i = 0; i < numRays; ++i) {
                rays.O[i] = origins[i];
                rays.D[i] = directions[i];
            }
            rays.normalize_directions();

            radiance = Vec<>::Constant(numRays, 1.0);
            totalDistance = Vec<>::Zero(numRays);
            latestDistance = Vec<>::Zero(numRays);
            latestCosine = Vec<>::Zero(numRays);
            latestPatchId = Vec<int>::Constant(numRays, -1);
            previousPatchId = Vec<int>::Constant(numRays, -1);
        }

        void RayBundle::traceAll(const TriangleMeshSoA& triangles)
        {
#if PROFILE_TRACE_ALL
			const auto startTime = std::chrono::high_resolution_clock::now();

			static double total = 0.0;
			static int count = 0;
#endif

#if USE_OMP_RAYTRACE_ALL
			constexpr int WorkerThreads = 8;
			#pragma omp parallel for num_threads(WorkerThreads)
#endif
            for (int i = 0; i < numRays; ++i) {
                // Skip rays that are already invalid.
                if (std::isnan(radiance(i)))
                    continue;

				// Buffers for ray processing
				int patchIdFront, patchIdBack;
				Real distanceFront, distanceBack, cosineFront, cosineBack;

                trace_ray(
                    triangles, rays, i,
                    patchIdFront, distanceFront, cosineFront,
                    patchIdBack, distanceBack, cosineBack,
                    latestPatchId(i));

                // NB: Don't mess up the order of operations!
                // Update previousPatchId before overwriting latestPatchId.
                previousPatchId(i) = latestPatchId(i);
                latestPatchId(i) = patchIdBack;
                latestDistance(i) = distanceFront;
                latestCosine(i) = cosineFront;
            }

#if PROFILE_TRACE_ALL
			const auto endTime = std::chrono::high_resolution_clock::now();
			const int uS = (int)std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
			total += uS;
			++count;
			std::cout << std::format("RayBundle::traceAll: {}uS, average: {:.0f}\n", uS, total / count);
#endif
        }

        void RayBundle::advanceAndReflect(const TriangleMeshSoA& triangles)
        {
            // TODO: Port definition from a different project, if we ever want to trace multiple reflection orders.
            
            rays.normalize_directions();
        }

        void RayBundle::getOrigins(std::vector<Vec3>& origins) const
        {
            RAC_DEBUG_ASSERT(ToInt(origins.size()) == numRays, "Origins size and numRays must be equal" + ToString(ToInt(origins.size())));
            for (int i = 0; i < numRays; ++i) {
                origins[i] = rays.O[i];
            }
        }

        void RayBundle::getDirections(std::vector<Vec3>& directions) const
        {
            RAC_DEBUG_ASSERT(ToInt(directions.size()) == numRays, "Directions size and numRays must be equal" + ToString(ToInt(directions.size())));
            for (int i = 0; i < numRays; ++i) {
                directions[i] = rays.D[i];
            }
        }

        // TODO: Can these not just use operator= ?
        void RayBundle::getTotalDistances(Vec<>& distances) const
        {
            RAC_DEBUG_ASSERT(distances.Length() == numRays, "Distances length and numRays must be equal" + ToString(distances.Length()));
            for (int i = 0; i < numRays; ++i)
                distances(i) = totalDistance(i);
        }

        void RayBundle::getCosines(Vec<>& cosines) const
        {
            RAC_DEBUG_ASSERT(cosines.Length() == numRays, "Cosines length and numRays must be equal" + ToString(cosines.Length()));
            for (int i = 0; i < numRays; ++i)
                cosines(i) = latestCosine(i);
        }

        void RayBundle::getIndices(Vec<int>& current, Vec<int>& previous) const
        {
            RAC_DEBUG_ASSERT(current.Length() == numRays, "Current length and numRays must be equal" + ToString(current.Length()));
            RAC_DEBUG_ASSERT(previous.Length() == numRays, "Previous length and numRays must be equal" + ToString(previous.Length()));
            for (int i = 0; i < numRays; ++i) {
                current(i) = latestPatchId(i);
                previous(i) = previousPatchId(i);
            }
        }

        void RayBundle::getRadiance(Vec<>& rad) const
        {
            RAC_DEBUG_ASSERT(rad.Length() == numRays, "Rad length and numRays must be equal" + ToString(rad.Length()));
            RAC_DEBUG_ASSERT(radiance.Length() == numRays, "Radiance length and numRays must be equal" + ToString(radiance.Length()));
            for (int i = 0; i < numRays; ++i)
                rad(i) = radiance(i);
        }

        // ------------------------ RayPencil methods ------------------------

        RayPencil::RayPencil()
        {
            numRays = 0;
            rays.resize(0);

            frontDistance = Vec<>::Zero(numRays);
            backDistance = Vec<>::Zero(numRays);
            frontCosine = Vec<>::Zero(numRays);
            backCosine = Vec<>::Zero(numRays);
            frontpatchId = Vec<int>::Constant(numRays, -1);
            backPatchId = Vec<int>::Constant(numRays, -1);
        }

        RayPencil::RayPencil(int numDirections, bool hemisphereOnly)
        {
            numRays = numDirections;
            rays.resize(numRays);

            exposeMirrorCopies = hemisphereOnly;

            rays.O = Vec3(0.0, 0.0, 0.0);
            // Note that this automatically normalizes the directions.
            rays.fill_uniform_sphere(hemisphereOnly);

            frontDistance = Vec<>::Zero(numRays);
            backDistance = Vec<>::Zero(numRays);
            frontCosine = Vec<>::Zero(numRays);
            backCosine = Vec<>::Zero(numRays);
            frontpatchId = Vec<int>::Constant(numRays, -1);
            backPatchId = Vec<int>::Constant(numRays, -1);
        }

        RayPencil::RayPencil(const std::vector<Vec3>& directions)
        {
            numRays = ToInt(directions.size());
            rays.resize(numRays);

            rays.O = Vec3(0.0, 0.0, 0.0);
            for (int i = 0; i < numRays; ++i) {
                rays.D[i] = directions[i];
            }
            rays.normalize_directions();

            frontDistance = Vec<>::Zero(numRays);
            backDistance = Vec<>::Zero(numRays);
            frontCosine = Vec<>::Zero(numRays);
            backCosine = Vec<>::Zero(numRays);
            frontpatchId = Vec<int>::Constant(numRays, -1);
            backPatchId = Vec<int>::Constant(numRays, -1);
        }

        void RayPencil::moveOrigin(const Vec3& origin)
        {
            rays.O = origin;

            frontDistance = Vec<>::Zero(numRays);
            backDistance = Vec<>::Zero(numRays);
            frontCosine = Vec<>::Zero(numRays);
            backCosine = Vec<>::Zero(numRays);
            frontpatchId = Vec<int>::Constant(numRays, -1);
            backPatchId = Vec<int>::Constant(numRays, -1);
        }

        void RayPencil::traceAll(const TriangleMeshSoA& triangles)
        {
#if PROFILE_TRACE_ALL
			const auto startTime = std::chrono::high_resolution_clock::now();

			static double total = 0.0;
			static int count = 0;
#endif
            
#if USE_OMP_RAYTRACE_ALL
            constexpr int WorkerThreads = 8;
			#pragma omp parallel for num_threads(WorkerThreads)
#endif
            for (int i = 0; i < numRays; ++i) {
				// Buffers for ray processing
				int temp_frontpatchId, temp_backpatchId;
				Real temp_frontDistance, temp_backDistance, temp_frontCosine, temp_backCosine;

                trace_ray(
                    triangles, rays, i,
                    temp_frontpatchId, temp_frontDistance, temp_frontCosine,
                    temp_backpatchId, temp_backDistance, temp_backCosine);

                frontpatchId(i) = temp_frontpatchId;
                frontDistance(i) = temp_frontDistance;
                frontCosine(i) = temp_frontCosine;
                backPatchId(i) = temp_backpatchId;
                backDistance(i) = temp_backDistance;
                backCosine(i) = temp_backCosine;
            }

#if PROFILE_TRACE_ALL
			const auto endTime = std::chrono::high_resolution_clock::now();
            const int uS = (int)std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
            total += uS;
            ++count;
            std::cout << std::format("RayPencil::traceAll: {}uS, average: {:.0f}\n", uS, total / count);
#endif
        }
        
        void RayPencil::clusterDirections(
            const std::vector<Vec3>& directions,
            Vec<int>& clusters) const
        {
            if (exposeMirrorCopies)
                RAC_DEBUG_ASSERT(clusters.Length() == 2 * numRays, "Clusters length must equal 2 * numRays" + ToString(clusters.Length()));
            else
                RAC_DEBUG_ASSERT(clusters.Length() == numRays, "Clusters length must equal numRays" + ToString(clusters.Length()));

            // Buffer used while iterating
            std::vector<Real> cosineSimilarity(directions.size());

            for (int i = 0; i < numRays; ++i)
            {
                if (directions.size() > 0)
                {
                    for (int j = 0; j < directions.size(); ++j)
                    {
                        // N.B.: Ray directions are guaranteed to have unit norm, cluster directions are ASSUMED to have unit norm.
                        // If cluster directions have non-unit norms, it will result in "weighted" clustering.
                        cosineSimilarity[j] = rays.D[i].dot(directions[j]);
                    }
                    clusters(i) = static_cast<int>( std::distance(cosineSimilarity.begin(), std::max_element(cosineSimilarity.begin(), cosineSimilarity.end())) );
                }
                else
                    clusters(i) = -1;
            }

            if (exposeMirrorCopies)
            {
                // Append direct opposites
                for (int i = 0; i < numRays; ++i)
                {
                    if (directions.size() > 0)
                    {
                        for (int j = 0; j < directions.size(); ++j)
                        {
                            // N.B.: Ray directions are guaranteed to have unit norm, cluster directions are ASSUMED to have unit norm.
                            // If cluster directions have non-unit norms, it will result in "weighted" clustering.
                            cosineSimilarity[j] = rays.D[i].dot(directions[j]);
                        }
                        // N.B.: We look for the MINIMUM this time
                        clusters(i + numRays) = static_cast<int>( std::distance(cosineSimilarity.begin(), std::min_element(cosineSimilarity.begin(), cosineSimilarity.end())) );
                    }
                    else
                        clusters(i + numRays) = -1;
                }
            }
        }

        void RayPencil::getDirections(std::vector<Vec3>& directions) const
        {
            if (exposeMirrorCopies)
                RAC_DEBUG_ASSERT(ToInt(directions.size()) == 2 * numRays, "Directions size must equal 2 * numRays" + ToString(ToInt(directions.size())));
            else
                RAC_DEBUG_ASSERT(ToInt(directions.size()) == numRays, "Directions size must equal numRays" + ToString(ToInt(directions.size())));

            for (int i = 0; i < numRays; ++i) {
                directions[i] = rays.D[i];
            }

            if (exposeMirrorCopies)
            {
                // Append direct opposites
                for (int i = 0; i < numRays; ++i)
                    directions[i + numRays] = -directions[i];
            }
        }

        void RayPencil::getDistances(Vec<>& distances) const
        {
            if (exposeMirrorCopies)
                RAC_DEBUG_ASSERT(distances.Length() == 2 * numRays, "Distances length must equal 2 * numRays" + ToString(distances.Length()));
            else
                RAC_DEBUG_ASSERT(distances.Length() == numRays, "Distances length must equal numRays" + ToString(distances.Length()));

            for (int i = 0; i < numRays; ++i)
                distances(i) = frontDistance(i);

            if (exposeMirrorCopies)
            {
                // Append direct opposites
                for (int i = 0; i < numRays; ++i)
                    distances(i + numRays) = backDistance(i);
            }
        }

        void RayPencil::getCosines(Vec<>& cosines) const
        {
            if (exposeMirrorCopies)
                RAC_DEBUG_ASSERT(cosines.Length() == 2 * numRays, "Cosines length must equal 2 * numRays" + ToString(cosines.Length()));
            else
                RAC_DEBUG_ASSERT(cosines.Length() == numRays, "Cosines length must equal numRays" + ToString(cosines.Length()));

            for (int i = 0; i < numRays; ++i)
                cosines(i) = frontCosine(i);

            if (exposeMirrorCopies)
            {
                // Append direct opposites
                for (int i = 0; i < numRays; ++i)
                    cosines(i + numRays) = backCosine(i);
            }
        }

        void RayPencil::getIndices(Vec<int>& front, Vec<int>& back) const
        {
            if (exposeMirrorCopies)
            {
                RAC_DEBUG_ASSERT(front.Length() == 2 * numRays, "Front length must equal 2 * numRays" + ToString(front.Length()));
                RAC_DEBUG_ASSERT(back.Length() == 2 * numRays, "Back length must equal 2 * numRays" + ToString(back.Length()));
            }
            else
            {
                RAC_DEBUG_ASSERT(front.Length() == numRays, "Front length must equal numRays" + ToString(front.Length()));
                RAC_DEBUG_ASSERT(back.Length() == numRays, "Back length must equal numRays" + ToString(back.Length()));
            }

            for (int i = 0; i < numRays; ++i)
            {
                front(i) = frontpatchId(i);
                back(i) = backPatchId(i);
            }

            if (exposeMirrorCopies)
            {
                // Append direct opposites
                for (int i = 0; i < numRays; ++i)
                {
                    front(i + numRays) = backPatchId(i);
                    back(i + numRays) = frontpatchId(i);
                }
            }
        }
	}
}
