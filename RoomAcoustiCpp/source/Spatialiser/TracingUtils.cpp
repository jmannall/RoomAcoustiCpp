
#include <cassert>
#include "Spatialiser/TracingUtils.h"

namespace RAC
{
    using namespace Common;
    namespace Spatialiser
    {
        // ------------------------ Intersection kernels ------------------------

        bool intersection_test_internal(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const Vec3& O, const Vec3& D, const Vec3& M,
            Real& distance, Real& cosine)
        {
			// Users should test the return result, but default the results to qNaN just in case
			distance = qNaN;
			cosine = qNaN;

			// Sanity check: the requested triangle must exist.
			assert(triangleIndex < triangles.size());

			// Load plane data into locals.
			const Vec3 n = triangles.n[triangleIndex];
			const Real d0 = triangles.d0[triangleIndex];

#if PLUCKER_KERNEL
#if LEAN_PLUCKER
			// TODO: Implement Lean Plücker
#else // not LEAN_PLUCKER
			// Load "fat Plücker" triangle data into locals.
			const Vec3& edgeABDirection = triangles.edgeABDirection[triangleIndex];
			const Vec3& edgeBCDirection = triangles.edgeBCDirection[triangleIndex];
			const Vec3& edgeCADirection = triangles.edgeCADirection[triangleIndex];
			const Vec3& edgeABWedge_AcrossB = triangles.edgeABWedge_AcrossB[triangleIndex];
			const Vec3& edgeBCWedge_BcrossC = triangles.edgeBCWedge_BcrossC[triangleIndex];
			const Vec3& edgeCAWedge_CcrossA = triangles.edgeCAWedge_CcrossA[triangleIndex];
			// ---------------------------------------------------------------------
			// 1) Facing test: ensure the triangle faces the ray origin.
			//    faceNum = dot(n, O) - d0; require faceNum > eps_face
			// ---------------------------------------------------------------------
			const Real faceNum = n.dot(O) - d0;
			if (faceNum < EPS_FACING) {
				return false;
			}

			// ---------------------------------------------------------------------
			// 2) Plücker side predicates for the three edges.
			//    For an edge PQ with direction e = Q - P and wedge W = P × Q,
			//    side = dot(D, W) + dot(M, e), where M = O × D (precomputed).
			//
			//    We inline the dot products (a0*b0 + a1*b1 + a2*b2) to avoid the
			//    overhead of tiny helper calls in this scalar, hot function.
			// ---------------------------------------------------------------------
			const Real sAB = D.dot(edgeABWedge_AcrossB) +
				M.dot(edgeABDirection);

			const Real sBC = D.dot(edgeBCWedge_BcrossC) +
				M.dot(edgeBCDirection);

			// ---------------------------------------------------------------------
			// Early-out on mismatched signs for the first two edges.
			// This matches the original logic exactly (edges included via epsilon).
			// ---------------------------------------------------------------------
			{
				const bool bothNonNeg = (sAB >= -EPS_EDGE) && (sBC >= -EPS_EDGE);
				const bool bothNonPos = (sAB <= EPS_EDGE) && (sBC <= EPS_EDGE);
				if (!(bothNonNeg || bothNonPos)) {
					return false;
				}
			}

			const Real sCA =
				D.dot(edgeCAWedge_CcrossA) +
				M.dot(edgeCADirection);

			// Keep the exact inclusion rule (edges included) via the provided helper.
			if (!same_sign_with_zero_included(sAB, sBC, sCA, EPS_EDGE)) {
				return false;
			}

			// ---------------------------------------------------------------------
			// 3) Compute the line parameter t with the triangle plane.
			//    No t>0 constraint (this is line–triangle, not ray–triangle).
			//    If the line is near-parallel to the plane, report no hit (NaN).
			// ---------------------------------------------------------------------
			const Real denom = n.dot(D);        // dot(n, D)
			if (std::abs(denom) <= EPS_PARALLEL) {
				return false;
			}
			else {
				distance = -faceNum / denom;
				cosine = std::abs(denom);
			}
			return true;
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNEL
			// Load "Möller–Trumbore" triangle data into locals.
			const Vec3& A = triangles.A[triangleIndex];
			const Vec3& e1 = triangles.edge1[triangleIndex];
			const Vec3& e2 = triangles.edge2[triangleIndex];

			// ---------------------------------------------------------------------
			// 1) Facing test.
			//    faceNum = dot(n, O) - d0 > eps_face
			// ---------------------------------------------------------------------
			const Real faceNum = n.dot(O) - d0;
			if (faceNum < EPS_FACING) {
				return false;
			}

			// ---------------------------------------------------------------------
			// 2) Möller–Trumbore barycentric numerators (unnormalized).
			//
			//    pvec = D × e2
			//    det  = e1 · pvec  (also equals dot(n, D))
			//    tvec = O - A
			//    u_num = dot(tvec, pvec)
			//    qvec  = tvec × e1
			//    v_num = dot(D, qvec)
			//    w_num = det - u_num - v_num   (since u+v+w = 1)
			//
			// We perform *sign* checks on (u_num, v_num, w_num) with an absolute
			// epsilon, for inclusive-edge behavior (no division by det => no sign flip issues).
			// ---------------------------------------------------------------------

			// pvec = D × e2
			const Vec3 pvec = D.cross(e2);

			// det = e1 · pvec  (also equals dot(n, D))
			const Real det = e1.dot(pvec);

			// tvec = O - A
			const Vec3 tvec = O - A;

			// u_num = dot(tvec, pvec)
			const Real u_num = tvec.dot(pvec);

			// qvec = tvec × e1
			const Vec3 qvec = tvec.cross(e1);

			// v_num = dot(D, qvec)
			const Real v_num = D.dot(qvec);

			// Early-out on u & v having opposite signs (edges included via epsilon).
			{
				const bool bothNonNeg = (u_num >= -EPS_EDGE) && (v_num >= -EPS_EDGE);
				const bool bothNonPos = (u_num <= EPS_EDGE) && (v_num <= EPS_EDGE);
				if (!(bothNonNeg || bothNonPos)) {
					return false;
				}
			}

			// w_num = det - u_num - v_num
			const Real w_num = det - (u_num + v_num);

			// All three barycentric numerators must share the same sign (edges included).
			if (!same_sign_with_zero_included(u_num, v_num, w_num, EPS_EDGE)) {
				return false;
			}

			// ---------------------------------------------------------------------
			// 3) Parallel test + compute t.
			//    If |det| is tiny, treat as parallel (report no hit).
			//    Otherwise: t = (e2 · qvec) / det
			//
			// NOTE: We postpone the parallel test until here to mirror the control
			// flow of your Plücker kernel (inside first, then parallel); we never
			// divide by det before checking it.
			// ---------------------------------------------------------------------
			if (std::abs(det) <= EPS_PARALLEL) {
				return false;
			}
			else {
				const Real t_num = e2.dot(qvec);; // e2 · qvec
				distance = t_num / det;
				cosine = std::abs(n.dot(D));
			}
			return true;
#endif // end PLUCKER_KERNEL
        }


        bool intersection_test(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const RayBundleSoA& rays, int rayIndex,
            Real& distance, Real& cosine)
        {
            // Sanity check: the requested ray must exist.
            assert(rayIndex < rays.size());

            // Load ray data into locals.
            const Vec3& O = rays.O[rayIndex];
            const Vec3& D = rays.D[rayIndex];

#if PLUCKER_KERNEL
			const Vec3& M = rays.M[rayIndex];
#else
            static Vec3 M(0.0, 0.0, 0.0);
#endif

            return intersection_test_internal(triangles, triangleIndex, O, D, M, distance, cosine);
        }

        bool intersection_test(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const RayPencilSoA& rays, int rayIndex,
            Real& distance, Real& cosine)
        {
            // Sanity check: the requested ray must exist.
            assert(rayIndex < rays.size());

            // Load ray data into locals.
            const Vec3& O = rays.O;
        	const Vec3& D = rays.D[rayIndex];

#if PLUCKER_KERNEL
#if LEAN_PLUCKER
            // TODO: Implement Lean Plücker
#else // not LEAN_PLUCKER
            // Load ray moments into locals.
            const Vec3 &M = rays.M[rayIndex];

#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNEL
            static const Vec3 M(0.0, 0.0, 0.0);
#endif

			return intersection_test_internal(triangles, triangleIndex, O, D, M, distance, cosine);
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

#if PLUCKER_KERNEL
#if LEAN_PLUCKER
			// TODO: Implement Lean Plücker
#else // not LEAN_PLUCKER
			// Compute ray moments and load into locals.
			const Vec3 rayMoment = rayOrigin.cross(rayDirection);
			const Vec3& M = rayMoment;
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNE
			static const Vec3 M(0.0, 0.0, 0.0);
#endif // end PLUCKER_KERNEL

			return intersection_test_internal(triangles, triangleIndex, O, D, M, distance, cosine);
		}

        bool intersection_test(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const Vec3& rayOrigin, const Vec3& rayDirection,
            Real& distance, Real& cosine)
        {
            // Load ray data into locals.
            const Vec3& O = rayOrigin;
            const Vec3& D = rayDirection;

#if PLUCKER_KERNEL
#if LEAN_PLUCKER
            // TODO: Implement Lean Plücker
#else // not LEAN_PLUCKER
            // Compute ray moments and load into locals.
            const Vec3 rayMoment = rayOrigin.cross(rayDirection);
            const Vec3& M = rayMoment;
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNE
            static const Vec3 M(0.0, 0.0, 0.0);
#endif // end PLUCKER_KERNEL

			return intersection_test_internal(triangles, triangleIndex, O, D, M, distance, cosine);
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
			distanceFront = INFINITY;
			distanceBack = -INFINITY;
			cosineFront = qNaN;
			cosineBack = qNaN;

			// Buffers to retrieve individual check results.
			Real currentDist, currentCos;

			for (int i = 0; i < triangles.size(); ++i) {
				if (i == ignoredTriangleIndex) // Ignore this triangle
					continue;

				if (!intersection_test(triangles, i, rays, rayIndex, currentDist, currentCos))
					continue;

				if ((currentDist + EPS_ZFIGHT < distanceBack) || (currentDist - EPS_ZFIGHT > distanceFront))
					continue; // Outside of current best range
				if (std::abs(currentDist) < EPS_SELFHIT)
					continue; // Too close to origin

				// Valid hit
				if (currentDist > 0.0) {
					if (std::abs(currentDist - distanceFront) < EPS_ZFIGHT) {
						// Z-fighting, lower triangle index wins.
						if (i < patchIdFront) {
							patchIdFront = triangles.patchId[i];
							distanceFront = currentDist;
							cosineFront = currentCos;
						} // else {keep the previous best}
					}
					else {
						patchIdFront = triangles.patchId[i];
						distanceFront = currentDist;
						cosineFront = currentCos;
					}
				}
				else {
					if (std::abs(currentDist - distanceBack) < EPS_ZFIGHT) {
						// Z-fighting, lower triangle index wins
						if (i < patchIdFront) {
							patchIdBack = triangles.patchId[i];
							distanceBack = currentDist;
							cosineBack = currentCos;
						} // else {keep the previous best}
					}
					else {
						patchIdBack = triangles.patchId[i];
						distanceBack = currentDist;
						cosineBack = currentCos;
					}
				}
			}

			if (std::isinf(distanceFront)) {
				// If it's still the initial INFINITY value, there was no valid hit at all.
				patchIdFront = -1;
				distanceFront = qNaN;
				cosineFront = qNaN;
			}
			if (std::isinf(distanceBack)) {
				// If it's still the initial -INFINITY value, there was no valid hit at all.
				patchIdBack = -1;
				distanceBack = qNaN;
				cosineBack = qNaN;
			}
			else // If the distanceBack is valid, return its absolute value.
				distanceBack = -distanceBack;
        }


        // TODO: The three definitions of this overloaded function are identical except for one line. I'm sure there's a more elegant way to do that.
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
            // Note that this also computes the moments if needed.
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
            // Note that this also computes the moments if needed.
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
            // Buffers for ray processing
            int patchIdFront, patchIdBack;
            Real distanceFront, distanceBack, cosineFront, cosineBack;

            for (int i = 0; i < numRays; ++i) {
                // Skip rays that are already invalid.
                if (std::isnan(radiance(i)))
                    continue;

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
        }

        void RayBundle::advanceAndReflect(const TriangleMeshSoA& triangles)
        {
            // TODO: Port definition from a different project, if we ever want to trace multiple reflection orders.
            
            // Note that this also computes the moments if needed.
            rays.normalize_directions();
        }

        void RayBundle::getOrigins(std::vector<Vec3>& origins) const
        {
            assert(origins.size() == numRays);
            for (int i = 0; i < numRays; ++i) {
                origins[i] = rays.O[i];
            }
        }

        void RayBundle::getDirections(std::vector<Vec3>& directions) const
        {
            assert(directions.size() == numRays);
            for (int i = 0; i < numRays; ++i) {
                directions[i] = rays.D[i];
            }
        }

        // TODO: Can these not just use operator= ?
        void RayBundle::getTotalDistances(Vec<>& distances) const
        {
            assert(distances.Length() == numRays);
            for (int i = 0; i < numRays; ++i)
                distances(i) = totalDistance(i);
        }

        void RayBundle::getCosines(Vec<>& cosines) const
        {
            assert(cosines.Length() == numRays);
            for (int i = 0; i < numRays; ++i)
                cosines(i) = latestCosine(i);
        }

        void RayBundle::getIndices(Vec<int>& current, Vec<int>& previous) const
        {
            assert(current.Length() == numRays);
            assert(previous.Length() == numRays);
            for (int i = 0; i < numRays; ++i) {
                current(i) = latestPatchId(i);
                previous(i) = previousPatchId(i);
            }
        }

        void RayBundle::getRadiance(Vec<>& rad) const
        {
            assert(rad.Length() == numRays);
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
            backpatchId = Vec<int>::Constant(numRays, -1);
        }

        RayPencil::RayPencil(int numDirections, bool hemisphereOnly)
        {
            numRays = numDirections;
            rays.resize(numRays);

            exposeMirrorCopies = hemisphereOnly;

            rays.O = Vec3(0.0, 0.0, 0.0);
            // Note that this automatically normalizes the directions and computes the moments if needed.
            rays.fill_uniform_sphere(hemisphereOnly);

            frontDistance = Vec<>::Zero(numRays);
            backDistance = Vec<>::Zero(numRays);
            frontCosine = Vec<>::Zero(numRays);
            backCosine = Vec<>::Zero(numRays);
            frontpatchId = Vec<int>::Constant(numRays, -1);
            backpatchId = Vec<int>::Constant(numRays, -1);
        }

        RayPencil::RayPencil(const std::vector<Vec3>& directions)
        {
            numRays = ToInt(directions.size());
            rays.resize(numRays);

            rays.O = Vec3(0.0, 0.0, 0.0);
            for (int i = 0; i < numRays; ++i) {
                rays.D[i] = directions[i];
            }
            // Note that this also computes the moments if needed.
            rays.normalize_directions();

            frontDistance = Vec<>::Zero(numRays);
            backDistance = Vec<>::Zero(numRays);
            frontCosine = Vec<>::Zero(numRays);
            backCosine = Vec<>::Zero(numRays);
            frontpatchId = Vec<int>::Constant(numRays, -1);
            backpatchId = Vec<int>::Constant(numRays, -1);
        }

        void RayPencil::moveOrigin(const Vec3& origin)
        {
            rays.O.noalias() += origin;

#if PLUCKER_KERNEL
            rays.compute_moments();
#endif // end PLUCKER_KERNEL

            frontDistance = Vec<>::Zero(numRays);
            backDistance = Vec<>::Zero(numRays);
            frontCosine = Vec<>::Zero(numRays);
            backCosine = Vec<>::Zero(numRays);
            frontpatchId = Vec<int>::Constant(numRays, -1);
            backpatchId = Vec<int>::Constant(numRays, -1);
        }

        void RayPencil::traceAll(const TriangleMeshSoA& triangles)
        {
            // Buffers for ray processing
            int temp_frontpatchId, temp_backpatchId;
            Real temp_frontDistance, temp_backDistance, temp_frontCosine, temp_backCosine;

            for (int i = 0; i < numRays; ++i) {
                trace_ray(
                    triangles, rays, i,
                    temp_frontpatchId, temp_frontDistance, temp_frontCosine,
                    temp_backpatchId, temp_backDistance, temp_backCosine);

                frontpatchId(i) = temp_frontpatchId;
                frontDistance(i) = temp_frontDistance;
                frontCosine(i) = temp_frontCosine;
                backpatchId(i) = temp_backpatchId;
                backDistance(i) = temp_backDistance;
                backCosine(i) = temp_backCosine;
            }
        }
        
        void RayPencil::clusterDirections(
            const std::vector<Vec3>& directions,
            Vec<int>& clusters) const
        {
            if (exposeMirrorCopies)
                assert(clusters.Length() == 2 * numRays);
            else
                assert(clusters.Length() == numRays);

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
                assert(directions.size() == 2 * numRays);
            else
                assert(directions.size() == numRays);

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
                assert(distances.Length() == 2 * numRays);
            else
                assert(distances.Length() == numRays);

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
                assert(cosines.Length() == 2 * numRays);
            else
                assert(cosines.Length() == numRays);

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
                assert(front.Length() == 2 * numRays);
                assert(back.Length() == 2 * numRays);
            }
            else
            {
                assert(front.Length() == numRays);
                assert(back.Length() == numRays);
            }

            for (int i = 0; i < numRays; ++i)
            {
                front(i) = frontpatchId(i);
                back(i) = backpatchId(i);
            }

            if (exposeMirrorCopies)
            {
                // Append direct opposites
                for (int i = 0; i < numRays; ++i)
                {
                    front(i + numRays) = backpatchId(i);
                    back(i + numRays) = frontpatchId(i);
                }
            }
        }
	}
}
