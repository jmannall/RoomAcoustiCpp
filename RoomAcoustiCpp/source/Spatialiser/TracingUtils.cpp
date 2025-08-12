#pragma optimize ("", off)
#include "Spatialiser/TracingUtils.h"

namespace RAC
{
    using namespace Common;
	namespace Spatialiser
	{
        // ------------------------ Intersection kernels ------------------------
        
        void intersection_test(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const RayBundleSoA& rays, int rayIndex,
            Real& distance, Real& cosine) {
            // Sanity check: the requested triangle must exist.
            assert(triangleIndex < triangles.size());
            // Sanity check: the requested ray must exist.
            assert(rayIndex < rays.size());

            // Load ray data into locals.
            const Real Ox = rays.Ox[rayIndex];
            const Real Oy = rays.Oy[rayIndex];
            const Real Oz = rays.Oz[rayIndex];

            const Real Dx = rays.Dx[rayIndex];
            const Real Dy = rays.Dy[rayIndex];
            const Real Dz = rays.Dz[rayIndex];

            // Load plane data into locals.
            const Real nx = triangles.nx[triangleIndex];
            const Real ny = triangles.ny[triangleIndex];
            const Real nz = triangles.nz[triangleIndex];
            const Real d0 = triangles.d0[triangleIndex];

#if PLUCKER_KERNEL
#if LEAN_PLUCKER
            // TODO: Implement Lean Plücker
#else // not LEAN_PLUCKER
            // Load ray moments into locals.
            const Real Mx = rays.Mx[rayIndex];
            const Real My = rays.My[rayIndex];
            const Real Mz = rays.Mz[rayIndex];

            // Load "fat Plücker" triangle data into locals.
            const Real edgeABDirectionX = triangles.edgeABDirectionX[triangleIndex];
            const Real edgeABDirectionY = triangles.edgeABDirectionY[triangleIndex];
            const Real edgeABDirectionZ = triangles.edgeABDirectionZ[triangleIndex];

            const Real edgeBCDirectionX = triangles.edgeBCDirectionX[triangleIndex];
            const Real edgeBCDirectionY = triangles.edgeBCDirectionY[triangleIndex];
            const Real edgeBCDirectionZ = triangles.edgeBCDirectionZ[triangleIndex];

            const Real edgeCADirectionX = triangles.edgeCADirectionX[triangleIndex];
            const Real edgeCADirectionY = triangles.edgeCADirectionY[triangleIndex];
            const Real edgeCADirectionZ = triangles.edgeCADirectionZ[triangleIndex];

            const Real edgeABWedge_AcrossB_X = triangles.edgeABWedge_AcrossB_X[triangleIndex];
            const Real edgeABWedge_AcrossB_Y = triangles.edgeABWedge_AcrossB_Y[triangleIndex];
            const Real edgeABWedge_AcrossB_Z = triangles.edgeABWedge_AcrossB_Z[triangleIndex];

            const Real edgeBCWedge_BcrossC_X = triangles.edgeBCWedge_BcrossC_X[triangleIndex];
            const Real edgeBCWedge_BcrossC_Y = triangles.edgeBCWedge_BcrossC_Y[triangleIndex];
            const Real edgeBCWedge_BcrossC_Z = triangles.edgeBCWedge_BcrossC_Z[triangleIndex];

            const Real edgeCAWedge_CcrossA_X = triangles.edgeCAWedge_CcrossA_X[triangleIndex];
            const Real edgeCAWedge_CcrossA_Y = triangles.edgeCAWedge_CcrossA_Y[triangleIndex];
            const Real edgeCAWedge_CcrossA_Z = triangles.edgeCAWedge_CcrossA_Z[triangleIndex];

            // ---------------------------------------------------------------------
            // 1) Facing test: ensure the triangle faces the ray origin.
            //    faceNum = dot(n, O) - d0; require faceNum > eps_face
            // ---------------------------------------------------------------------
            const Real faceNum = nx * Ox + ny * Oy + nz * Oz - d0;
            if (faceNum < EPS_FACING) {
                distance = qNaN;
                cosine = qNaN;
                return;
            }

            // ---------------------------------------------------------------------
            // 2) Plücker side predicates for the three edges.
            //    For an edge PQ with direction e = Q - P and wedge W = P × Q,
            //    side = dot(D, W) + dot(M, e), where M = O × D (precomputed).
            //
            //    We inline the dot products (a0*b0 + a1*b1 + a2*b2) to avoid the
            //    overhead of tiny helper calls in this scalar, hot function.
            // ---------------------------------------------------------------------
            const Real sAB =
                (Dx * edgeABWedge_AcrossB_X + Dy * edgeABWedge_AcrossB_Y + Dz * edgeABWedge_AcrossB_Z) +
                (Mx * edgeABDirectionX + My * edgeABDirectionY + Mz * edgeABDirectionZ);

            const Real sBC =
                (Dx * edgeBCWedge_BcrossC_X + Dy * edgeBCWedge_BcrossC_Y + Dz * edgeBCWedge_BcrossC_Z) +
                (Mx * edgeBCDirectionX + My * edgeBCDirectionY + Mz * edgeBCDirectionZ);

            // ---------------------------------------------------------------------
            // Early-out on mismatched signs for the first two edges.
            // This matches the original logic exactly (edges included via epsilon).
            // ---------------------------------------------------------------------
            {
                const bool bothNonNeg = (sAB >= -EPS_EDGE) && (sBC >= -EPS_EDGE);
                const bool bothNonPos = (sAB <= EPS_EDGE) && (sBC <= EPS_EDGE);
                if (!(bothNonNeg || bothNonPos)) {
                    distance = qNaN;
                    cosine = qNaN;
                    return;
                }
            }

            const Real sCA =
                (Dx * edgeCAWedge_CcrossA_X + Dy * edgeCAWedge_CcrossA_Y + Dz * edgeCAWedge_CcrossA_Z) +
                (Mx * edgeCADirectionX + My * edgeCADirectionY + Mz * edgeCADirectionZ);

            // Keep the exact inclusion rule (edges included) via the provided helper.
            if (!same_sign_with_zero_included(sAB, sBC, sCA, EPS_EDGE)) {
                distance = qNaN;
                cosine = qNaN;
                return;
            }

            // ---------------------------------------------------------------------
            // 3) Compute the line parameter t with the triangle plane.
            //    No t>0 constraint (this is line–triangle, not ray–triangle).
            //    If the line is near-parallel to the plane, report no hit (NaN).
            // ---------------------------------------------------------------------
            const Real denom = nx * Dx + ny * Dy + nz * Dz;  // dot(n, D)
            if (std::abs(denom) <= EPS_PARALLEL) {
                distance = qNaN;
                cosine = qNaN;
            }
            else {
                distance = -faceNum / denom;
                cosine = std::abs(denom);
            }
            return;
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNEL
            // Load "Möller–Trumbore" triangle data into locals.
            const Real Ax = triangles.Ax[triangleIndex];
            const Real Ay = triangles.Ay[triangleIndex];
            const Real Az = triangles.Az[triangleIndex];

            const Real e1x = triangles.edge1X[triangleIndex];
            const Real e1y = triangles.edge1Y[triangleIndex];
            const Real e1z = triangles.edge1Z[triangleIndex];

            const Real e2x = triangles.edge2X[triangleIndex];
            const Real e2y = triangles.edge2Y[triangleIndex];
            const Real e2z = triangles.edge2Z[triangleIndex];

            // ---------------------------------------------------------------------
            // 1) Facing test.
            //    faceNum = dot(n, O) - d0 > eps_face
            // ---------------------------------------------------------------------
            const Real faceNum = nx * Ox + ny * Oy + nz * Oz - d0;
            if (faceNum < EPS_FACING) {
                distance = qNaN;
                cosine = qNaN;
                return;
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
            const Real pvec_x = Dy * e2z - Dz * e2y;
            const Real pvec_y = Dz * e2x - Dx * e2z;
            const Real pvec_z = Dx * e2y - Dy * e2x;

            // det = e1 · pvec  (also equals dot(n, D))
            const Real det = e1x * pvec_x + e1y * pvec_y + e1z * pvec_z;

            // tvec = O - A
            const Real tvec_x = Ox - Ax;
            const Real tvec_y = Oy - Ay;
            const Real tvec_z = Oz - Az;

            // u_num = dot(tvec, pvec)
            const Real u_num = tvec_x * pvec_x + tvec_y * pvec_y + tvec_z * pvec_z;

            // qvec = tvec × e1
            const Real qvec_x = tvec_y * e1z - tvec_z * e1y;
            const Real qvec_y = tvec_z * e1x - tvec_x * e1z;
            const Real qvec_z = tvec_x * e1y - tvec_y * e1x;

            // v_num = dot(D, qvec)
            const Real v_num = Dx * qvec_x + Dy * qvec_y + Dz * qvec_z;

            // Early-out on u & v having opposite signs (edges included via epsilon).
            {
                const bool bothNonNeg = (u_num >= -EPS_EDGE) && (v_num >= -EPS_EDGE);
                const bool bothNonPos = (u_num <= EPS_EDGE) && (v_num <= EPS_EDGE);
                if (!(bothNonNeg || bothNonPos)) {
                    distance = qNaN;
                    cosine = qNaN;
                    return;
                }
            }

            // w_num = det - u_num - v_num
            const Real w_num = det - (u_num + v_num);

            // All three barycentric numerators must share the same sign (edges included).
            if (!same_sign_with_zero_included(u_num, v_num, w_num, EPS_EDGE)) {
                distance = qNaN;
                cosine = qNaN;
                return;
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
                distance = qNaN;
                cosine = qNaN;
            }
            else {
                const Real t_num = e2x * qvec_x + e2y * qvec_y + e2z * qvec_z; // e2 · qvec
                distance = t_num / det;
                cosine = std::abs(nx * Dx + ny * Dy + nz * Dz);
            }
            return;
#endif // end PLUCKER_KERNEL
        }

        void intersection_test(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const RayPencilSoA& rays, int rayIndex,
            Real& distance, Real& cosine) {
            // Sanity check: the requested triangle must exist.
            assert(triangleIndex < triangles.size());
            // Sanity check: the requested ray must exist.
            assert(rayIndex < rays.size());

            // Load ray data into locals.
            const Real Ox = rays.Ox;
            const Real Oy = rays.Oy;
            const Real Oz = rays.Oz;

            const Real Dx = rays.Dx[rayIndex];
            const Real Dy = rays.Dy[rayIndex];
            const Real Dz = rays.Dz[rayIndex];

            // Load plane data into locals.
            const Real nx = triangles.nx[triangleIndex];
            const Real ny = triangles.ny[triangleIndex];
            const Real nz = triangles.nz[triangleIndex];
            const Real d0 = triangles.d0[triangleIndex];

#if PLUCKER_KERNEL
#if LEAN_PLUCKER
            // TODO: Implement Lean Plücker
#else // not LEAN_PLUCKER
            // Load ray moments into locals.
            const Real Mx = rays.Mx[rayIndex];
            const Real My = rays.My[rayIndex];
            const Real Mz = rays.Mz[rayIndex];

            // Load "fat Plücker" triangle data into locals.
            const Real edgeABDirectionX = triangles.edgeABDirectionX[triangleIndex];
            const Real edgeABDirectionY = triangles.edgeABDirectionY[triangleIndex];
            const Real edgeABDirectionZ = triangles.edgeABDirectionZ[triangleIndex];

            const Real edgeBCDirectionX = triangles.edgeBCDirectionX[triangleIndex];
            const Real edgeBCDirectionY = triangles.edgeBCDirectionY[triangleIndex];
            const Real edgeBCDirectionZ = triangles.edgeBCDirectionZ[triangleIndex];

            const Real edgeCADirectionX = triangles.edgeCADirectionX[triangleIndex];
            const Real edgeCADirectionY = triangles.edgeCADirectionY[triangleIndex];
            const Real edgeCADirectionZ = triangles.edgeCADirectionZ[triangleIndex];

            const Real edgeABWedge_AcrossB_X = triangles.edgeABWedge_AcrossB_X[triangleIndex];
            const Real edgeABWedge_AcrossB_Y = triangles.edgeABWedge_AcrossB_Y[triangleIndex];
            const Real edgeABWedge_AcrossB_Z = triangles.edgeABWedge_AcrossB_Z[triangleIndex];

            const Real edgeBCWedge_BcrossC_X = triangles.edgeBCWedge_BcrossC_X[triangleIndex];
            const Real edgeBCWedge_BcrossC_Y = triangles.edgeBCWedge_BcrossC_Y[triangleIndex];
            const Real edgeBCWedge_BcrossC_Z = triangles.edgeBCWedge_BcrossC_Z[triangleIndex];

            const Real edgeCAWedge_CcrossA_X = triangles.edgeCAWedge_CcrossA_X[triangleIndex];
            const Real edgeCAWedge_CcrossA_Y = triangles.edgeCAWedge_CcrossA_Y[triangleIndex];
            const Real edgeCAWedge_CcrossA_Z = triangles.edgeCAWedge_CcrossA_Z[triangleIndex];

            // ---------------------------------------------------------------------
            // 1) Facing test: ensure the triangle faces the ray origin.
            //    faceNum = dot(n, O) - d0; require faceNum > eps_face
            // ---------------------------------------------------------------------
            const Real faceNum = nx * Ox + ny * Oy + nz * Oz - d0;
            if (faceNum < EPS_FACING) {
                distance = qNaN;
                cosine = qNaN;
                return;
            }

            // ---------------------------------------------------------------------
            // 2) Plücker side predicates for the three edges.
            //    For an edge PQ with direction e = Q - P and wedge W = P × Q,
            //    side = dot(D, W) + dot(M, e), where M = O × D (precomputed).
            //
            //    We inline the dot products (a0*b0 + a1*b1 + a2*b2) to avoid the
            //    overhead of tiny helper calls in this scalar, hot function.
            // ---------------------------------------------------------------------
            const Real sAB =
                (Dx * edgeABWedge_AcrossB_X + Dy * edgeABWedge_AcrossB_Y + Dz * edgeABWedge_AcrossB_Z) +
                (Mx * edgeABDirectionX + My * edgeABDirectionY + Mz * edgeABDirectionZ);

            const Real sBC =
                (Dx * edgeBCWedge_BcrossC_X + Dy * edgeBCWedge_BcrossC_Y + Dz * edgeBCWedge_BcrossC_Z) +
                (Mx * edgeBCDirectionX + My * edgeBCDirectionY + Mz * edgeBCDirectionZ);

            // ---------------------------------------------------------------------
            // Early-out on mismatched signs for the first two edges.
            // This matches the original logic exactly (edges included via epsilon).
            // ---------------------------------------------------------------------
            {
                const bool bothNonNeg = (sAB >= -EPS_EDGE) && (sBC >= -EPS_EDGE);
                const bool bothNonPos = (sAB <= EPS_EDGE) && (sBC <= EPS_EDGE);
                if (!(bothNonNeg || bothNonPos)) {
                    distance = qNaN;
                    cosine = qNaN;
                    return;
                }
            }

            const Real sCA =
                (Dx * edgeCAWedge_CcrossA_X + Dy * edgeCAWedge_CcrossA_Y + Dz * edgeCAWedge_CcrossA_Z) +
                (Mx * edgeCADirectionX + My * edgeCADirectionY + Mz * edgeCADirectionZ);

            // Keep the exact inclusion rule (edges included) via the provided helper.
            if (!same_sign_with_zero_included(sAB, sBC, sCA, EPS_EDGE)) {
                distance = qNaN;
                cosine = qNaN;
                return;
            }

            // ---------------------------------------------------------------------
            // 3) Compute the line parameter t with the triangle plane.
            //    No t>0 constraint (this is line–triangle, not ray–triangle).
            //    If the line is near-parallel to the plane, report no hit (NaN).
            // ---------------------------------------------------------------------
            const Real denom = nx * Dx + ny * Dy + nz * Dz;  // dot(n, D)
            if (std::abs(denom) <= EPS_PARALLEL) {
                distance = qNaN;
                cosine = qNaN;
            }
            else {
                distance = -faceNum / denom;
                cosine = std::abs(denom);
            }
            return;
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNEL
            // Load "Möller–Trumbore" triangle data into locals.
            const Real Ax = triangles.Ax[triangleIndex];
            const Real Ay = triangles.Ay[triangleIndex];
            const Real Az = triangles.Az[triangleIndex];

            const Real e1x = triangles.edge1X[triangleIndex];
            const Real e1y = triangles.edge1Y[triangleIndex];
            const Real e1z = triangles.edge1Z[triangleIndex];

            const Real e2x = triangles.edge2X[triangleIndex];
            const Real e2y = triangles.edge2Y[triangleIndex];
            const Real e2z = triangles.edge2Z[triangleIndex];

            // ---------------------------------------------------------------------
            // 1) Facing test.
            //    faceNum = dot(n, O) - d0 > eps_face
            // ---------------------------------------------------------------------
            const Real faceNum = nx * Ox + ny * Oy + nz * Oz - d0;
            if (faceNum < EPS_FACING) {
                distance = qNaN;
                cosine = qNaN;
                return;
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
            const Real pvec_x = Dy * e2z - Dz * e2y;
            const Real pvec_y = Dz * e2x - Dx * e2z;
            const Real pvec_z = Dx * e2y - Dy * e2x;

            // det = e1 · pvec  (also equals dot(n, D))
            const Real det = e1x * pvec_x + e1y * pvec_y + e1z * pvec_z;

            // tvec = O - A
            const Real tvec_x = Ox - Ax;
            const Real tvec_y = Oy - Ay;
            const Real tvec_z = Oz - Az;

            // u_num = dot(tvec, pvec)
            const Real u_num = tvec_x * pvec_x + tvec_y * pvec_y + tvec_z * pvec_z;

            // qvec = tvec × e1
            const Real qvec_x = tvec_y * e1z - tvec_z * e1y;
            const Real qvec_y = tvec_z * e1x - tvec_x * e1z;
            const Real qvec_z = tvec_x * e1y - tvec_y * e1x;

            // v_num = dot(D, qvec)
            const Real v_num = Dx * qvec_x + Dy * qvec_y + Dz * qvec_z;

            // Early-out on u & v having opposite signs (edges included via epsilon).
            {
                const bool bothNonNeg = (u_num >= -EPS_EDGE) && (v_num >= -EPS_EDGE);
                const bool bothNonPos = (u_num <= EPS_EDGE) && (v_num <= EPS_EDGE);
                if (!(bothNonNeg || bothNonPos)) {
                    distance = qNaN;
                    cosine = qNaN;
                    return;
                }
            }

            // w_num = det - u_num - v_num
            const Real w_num = det - (u_num + v_num);

            // All three barycentric numerators must share the same sign (edges included).
            if (!same_sign_with_zero_included(u_num, v_num, w_num, EPS_EDGE)) {
                distance = qNaN;
                cosine = qNaN;
                return;
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
                distance = qNaN;
                cosine = qNaN;
            }
            else {
                const Real t_num = e2x * qvec_x + e2y * qvec_y + e2z * qvec_z; // e2 · qvec
                distance = t_num / det;
                cosine = std::abs(nx * Dx + ny * Dy + nz * Dz);
            }
            return;
#endif // end PLUCKER_KERNEL
        }

        void intersection_test(
            const TriangleMeshSoA& triangles, int triangleIndex,
            const Vec3& rayOrigin, const Vec3& rayDirection,
            Real& distance, Real& cosine) {
            // Sanity check: the requested triangle must exist.
            assert(triangleIndex < triangles.size());
            // Sanity check: the requested ray must exist.
            // assert(rayIndex < rays.size());

            // Load ray data into locals.
            const Real Ox = rayOrigin.x;
            const Real Oy = rayOrigin.y;
            const Real Oz = rayOrigin.z;

            const Real Dx = rayDirection.x;
            const Real Dy = rayDirection.y;
            const Real Dz = rayDirection.z;

            // Load plane data into locals.
            const Real nx = triangles.nx[triangleIndex];
            const Real ny = triangles.ny[triangleIndex];
            const Real nz = triangles.nz[triangleIndex];
            const Real d0 = triangles.d0[triangleIndex];

#if PLUCKER_KERNEL
#if LEAN_PLUCKER
            // TODO: Implement Lean Plücker
#else // not LEAN_PLUCKER
            // Compute ray moments and load into locals.
            const Vec3 rayMoment = Cross(rayOrigin, rayDirection);
            const Real Mx = rayMoment.x;
            const Real My = rayMoment.y;
            const Real Mz = rayMoment.z;

            // Load "fat Plücker" triangle data into locals.
            const Real edgeABDirectionX = triangles.edgeABDirectionX[triangleIndex];
            const Real edgeABDirectionY = triangles.edgeABDirectionY[triangleIndex];
            const Real edgeABDirectionZ = triangles.edgeABDirectionZ[triangleIndex];

            const Real edgeBCDirectionX = triangles.edgeBCDirectionX[triangleIndex];
            const Real edgeBCDirectionY = triangles.edgeBCDirectionY[triangleIndex];
            const Real edgeBCDirectionZ = triangles.edgeBCDirectionZ[triangleIndex];

            const Real edgeCADirectionX = triangles.edgeCADirectionX[triangleIndex];
            const Real edgeCADirectionY = triangles.edgeCADirectionY[triangleIndex];
            const Real edgeCADirectionZ = triangles.edgeCADirectionZ[triangleIndex];

            const Real edgeABWedge_AcrossB_X = triangles.edgeABWedge_AcrossB_X[triangleIndex];
            const Real edgeABWedge_AcrossB_Y = triangles.edgeABWedge_AcrossB_Y[triangleIndex];
            const Real edgeABWedge_AcrossB_Z = triangles.edgeABWedge_AcrossB_Z[triangleIndex];

            const Real edgeBCWedge_BcrossC_X = triangles.edgeBCWedge_BcrossC_X[triangleIndex];
            const Real edgeBCWedge_BcrossC_Y = triangles.edgeBCWedge_BcrossC_Y[triangleIndex];
            const Real edgeBCWedge_BcrossC_Z = triangles.edgeBCWedge_BcrossC_Z[triangleIndex];

            const Real edgeCAWedge_CcrossA_X = triangles.edgeCAWedge_CcrossA_X[triangleIndex];
            const Real edgeCAWedge_CcrossA_Y = triangles.edgeCAWedge_CcrossA_Y[triangleIndex];
            const Real edgeCAWedge_CcrossA_Z = triangles.edgeCAWedge_CcrossA_Z[triangleIndex];

            // ---------------------------------------------------------------------
            // 1) Facing test: ensure the triangle faces the ray origin.
            //    faceNum = dot(n, O) - d0; require faceNum > eps_face
            // ---------------------------------------------------------------------
            const Real faceNum = nx * Ox + ny * Oy + nz * Oz - d0;
            if (faceNum < EPS_FACING) {
                distance = qNaN;
                cosine = qNaN;
                return;
            }

            // ---------------------------------------------------------------------
            // 2) Plücker side predicates for the three edges.
            //    For an edge PQ with direction e = Q - P and wedge W = P × Q,
            //    side = dot(D, W) + dot(M, e), where M = O × D (precomputed).
            //
            //    We inline the dot products (a0*b0 + a1*b1 + a2*b2) to avoid the
            //    overhead of tiny helper calls in this scalar, hot function.
            // ---------------------------------------------------------------------
            const Real sAB =
                (Dx * edgeABWedge_AcrossB_X + Dy * edgeABWedge_AcrossB_Y + Dz * edgeABWedge_AcrossB_Z) +
                (Mx * edgeABDirectionX + My * edgeABDirectionY + Mz * edgeABDirectionZ);

            const Real sBC =
                (Dx * edgeBCWedge_BcrossC_X + Dy * edgeBCWedge_BcrossC_Y + Dz * edgeBCWedge_BcrossC_Z) +
                (Mx * edgeBCDirectionX + My * edgeBCDirectionY + Mz * edgeBCDirectionZ);

            // ---------------------------------------------------------------------
            // Early-out on mismatched signs for the first two edges.
            // This matches the original logic exactly (edges included via epsilon).
            // ---------------------------------------------------------------------
            {
                const bool bothNonNeg = (sAB >= -EPS_EDGE) && (sBC >= -EPS_EDGE);
                const bool bothNonPos = (sAB <= EPS_EDGE) && (sBC <= EPS_EDGE);
                if (!(bothNonNeg || bothNonPos)) {
                    distance = qNaN;
                    cosine = qNaN;
                    return;
                }
            }

            const Real sCA =
                (Dx * edgeCAWedge_CcrossA_X + Dy * edgeCAWedge_CcrossA_Y + Dz * edgeCAWedge_CcrossA_Z) +
                (Mx * edgeCADirectionX + My * edgeCADirectionY + Mz * edgeCADirectionZ);

            // Keep the exact inclusion rule (edges included) via the provided helper.
            if (!same_sign_with_zero_included(sAB, sBC, sCA, EPS_EDGE)) {
                distance = qNaN;
                cosine = qNaN;
                return;
            }

            // ---------------------------------------------------------------------
            // 3) Compute the line parameter t with the triangle plane.
            //    No t>0 constraint (this is line–triangle, not ray–triangle).
            //    If the line is near-parallel to the plane, report no hit (NaN).
            // ---------------------------------------------------------------------
            const Real denom = nx * Dx + ny * Dy + nz * Dz;  // dot(n, D)
            if (std::abs(denom) <= EPS_PARALLEL) {
                distance = qNaN;
                cosine = qNaN;
            }
            else {
                distance = -faceNum / denom;
                cosine = std::abs(denom);
            }
            return;
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNEL
            // Load "Möller–Trumbore" triangle data into locals.
            const Real Ax = triangles.Ax[triangleIndex];
            const Real Ay = triangles.Ay[triangleIndex];
            const Real Az = triangles.Az[triangleIndex];

            const Real e1x = triangles.edge1X[triangleIndex];
            const Real e1y = triangles.edge1Y[triangleIndex];
            const Real e1z = triangles.edge1Z[triangleIndex];

            const Real e2x = triangles.edge2X[triangleIndex];
            const Real e2y = triangles.edge2Y[triangleIndex];
            const Real e2z = triangles.edge2Z[triangleIndex];

            // ---------------------------------------------------------------------
            // 1) Facing test.
            //    faceNum = dot(n, O) - d0 > eps_face
            // ---------------------------------------------------------------------
            const Real faceNum = nx * Ox + ny * Oy + nz * Oz - d0;
            if (faceNum < EPS_FACING) {
                distance = qNaN;
                cosine = qNaN;
                return;
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
            const Real pvec_x = Dy * e2z - Dz * e2y;
            const Real pvec_y = Dz * e2x - Dx * e2z;
            const Real pvec_z = Dx * e2y - Dy * e2x;

            // det = e1 · pvec  (also equals dot(n, D))
            const Real det = e1x * pvec_x + e1y * pvec_y + e1z * pvec_z;

            // tvec = O - A
            const Real tvec_x = Ox - Ax;
            const Real tvec_y = Oy - Ay;
            const Real tvec_z = Oz - Az;

            // u_num = dot(tvec, pvec)
            const Real u_num = tvec_x * pvec_x + tvec_y * pvec_y + tvec_z * pvec_z;

            // qvec = tvec × e1
            const Real qvec_x = tvec_y * e1z - tvec_z * e1y;
            const Real qvec_y = tvec_z * e1x - tvec_x * e1z;
            const Real qvec_z = tvec_x * e1y - tvec_y * e1x;

            // v_num = dot(D, qvec)
            const Real v_num = Dx * qvec_x + Dy * qvec_y + Dz * qvec_z;

            // Early-out on u & v having opposite signs (edges included via epsilon).
            {
                const bool bothNonNeg = (u_num >= -EPS_EDGE) && (v_num >= -EPS_EDGE);
                const bool bothNonPos = (u_num <= EPS_EDGE) && (v_num <= EPS_EDGE);
                if (!(bothNonNeg || bothNonPos)) {
                    distance = qNaN;
                    cosine = qNaN;
                    return;
                }
            }

            // w_num = det - u_num - v_num
            const Real w_num = det - (u_num + v_num);

            // All three barycentric numerators must share the same sign (edges included).
            if (!same_sign_with_zero_included(u_num, v_num, w_num, EPS_EDGE)) {
                distance = qNaN;
                cosine = qNaN;
                return;
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
                distance = qNaN;
                cosine = qNaN;
            }
            else {
                const Real t_num = e2x * qvec_x + e2y * qvec_y + e2z * qvec_z; // e2 · qvec
                distance = t_num / det;
                cosine = std::abs(nx * Dx + ny * Dy + nz * Dz);
            }
            return;
#endif // end PLUCKER_KERNEL
        }

        // ------------------------ Tracing loop kernels ------------------------

        // TODO: The three definitions of this overloaded function are identical except for one line. I'm sure there's a more elegant way to do that.
        void trace_ray(
            const TriangleMeshSoA& triangles, const RayBundleSoA& rays, int rayIndex,
            int& triangleIdxFront, Real& distanceFront, Real& cosineFront,
            int& triangleIdxBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex) {
            // Initialize per-ray front/back bests.
            triangleIdxFront = -1;
            triangleIdxBack = -1;
            distanceFront = INFINITY;
            distanceBack = -INFINITY;
            cosineFront = qNaN;
            cosineBack = qNaN;

            // Buffers to retrieve individual check results.
            Real currentDist, currentCos;

            for (int i = 0; i < triangles.size(); ++i) {
                if (i == ignoredTriangleIndex) // Ignore this triangle
                    continue;

                intersection_test(triangles, i, rays, rayIndex, currentDist, currentCos);

                if (std::isnan(currentDist))
                    continue; // Invalid hit
                if ((currentDist + EPS_ZFIGHT < distanceBack) || (currentDist - EPS_ZFIGHT > distanceFront))
                    continue; // Outside of current best range
                if (std::abs(currentDist) < EPS_SELFHIT)
                    continue; // Too close to origin

                // Valid hit
                if (currentDist > 0.0) {
                    if (std::abs(currentDist - distanceFront) < EPS_ZFIGHT) {
                        // Z-fighting, lower triangle index wins.
                        if (i < triangleIdxFront) {
                            triangleIdxFront = i;
                            distanceFront = currentDist;
                            cosineFront = currentCos;
                        } // else {keep the previous best}
                    }
                    else {
                        triangleIdxFront = i;
                        distanceFront = currentDist;
                        cosineFront = currentCos;
                    }
                }
                else {
                    if (std::abs(currentDist - distanceBack) < EPS_ZFIGHT) {
                        // Z-fighting, lower triangle index wins
                        if (i < triangleIdxFront) {
                            triangleIdxBack = i;
                            distanceBack = currentDist;
                            cosineBack = currentCos;
                        } // else {keep the previous best}
                    }
                    else {
                        triangleIdxBack = i;
                        distanceBack = currentDist;
                        cosineBack = currentCos;
                    }
                }
            }
        }

        void trace_ray(
            const TriangleMeshSoA& triangles, const RayPencilSoA& rays, int rayIndex,
            int& triangleIdxFront, Real& distanceFront, Real& cosineFront,
            int& triangleIdxBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex) {
            // Initialize per-ray front/back bests.
            triangleIdxFront = -1;
            triangleIdxBack = -1;
            distanceFront = INFINITY;
            distanceBack = -INFINITY;
            cosineFront = qNaN;
            cosineBack = qNaN;

            // Buffers to retrieve individual check results.
            Real currentDist, currentCos;

            for (int i = 0; i < triangles.size(); ++i) {
                if (i == ignoredTriangleIndex) // Ignore this triangle
                    continue;

                intersection_test(triangles, i, rays, rayIndex, currentDist, currentCos);

                if (std::isnan(currentDist))
                    continue; // Invalid hit
                if ((currentDist + EPS_ZFIGHT < distanceBack) || (currentDist - EPS_ZFIGHT > distanceFront))
                    continue; // Outside of current best range
                if (std::abs(currentDist) < EPS_SELFHIT)
                    continue; // Too close to origin

                // Valid hit
                if (currentDist > 0.0) {
                    if (std::abs(currentDist - distanceFront) < EPS_ZFIGHT) {
                        // Z-fighting, lower triangle index wins.
                        if (i < triangleIdxFront) {
                            triangleIdxFront = i;
                            distanceFront = currentDist;
                            cosineFront = currentCos;
                        } // else {keep the previous best}
                    }
                    else {
                        triangleIdxFront = i;
                        distanceFront = currentDist;
                        cosineFront = currentCos;
                    }
                }
                else {
                    if (std::abs(currentDist - distanceBack) < EPS_ZFIGHT) {
                        // Z-fighting, lower triangle index wins
                        if (i < triangleIdxFront) {
                            triangleIdxBack = i;
                            distanceBack = currentDist;
                            cosineBack = currentCos;
                        } // else {keep the previous best}
                    }
                    else {
                        triangleIdxBack = i;
                        distanceBack = currentDist;
                        cosineBack = currentCos;
                    }
                }
            }

            if (std::isinf(distanceFront)) {
                // If it's still the initial INFINITY value, there was no valid hit at all.
                triangleIdxFront = -1;
                distanceFront = qNaN;
                cosineFront = qNaN;
            }
            if (std::isinf(distanceBack)) {
                // If it's still the initial -INFINITY value, there was no valid hit at all.
                triangleIdxBack = -1;
                distanceBack = qNaN;
                cosineBack = qNaN;
            }
            else // If the distanceBack is valid, return its absolute value.
                distanceBack = -distanceBack;
        }

        void trace_ray(
            const TriangleMeshSoA& triangles, const Vec3& rayOrigin, const Vec3& rayDirection,
            int& triangleIdxFront, Real& distanceFront, Real& cosineFront,
            int& triangleIdxBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex) {
            // Initialize per-ray front/back bests.
            triangleIdxFront = -1;
            triangleIdxBack = -1;
            distanceFront = INFINITY;
            distanceBack = -INFINITY;
            cosineFront = qNaN;
            cosineBack = qNaN;

            // Buffers to retrieve individual check results.
            Real currentDist, currentCos;

            for (int i = 0; i < triangles.size(); ++i) {
                if (i == ignoredTriangleIndex) // Ignore this triangle
                    continue;

                intersection_test(triangles, i, rayOrigin, rayDirection, currentDist, currentCos);

                if (std::isnan(currentDist))
                    continue; // Invalid hit
                if ((currentDist + EPS_ZFIGHT < distanceBack) || (currentDist - EPS_ZFIGHT > distanceFront))
                    continue; // Outside of current best range
                if (std::abs(currentDist) < EPS_SELFHIT)
                    continue; // Too close to origin

                // Valid hit
                if (currentDist > 0.0) {
                    if (std::abs(currentDist - distanceFront) < EPS_ZFIGHT) {
                        // Z-fighting, lower triangle index wins.
                        if (i < triangleIdxFront) {
                            triangleIdxFront = i;
                            distanceFront = currentDist;
                            cosineFront = currentCos;
                        } // else {keep the previous best}
                    }
                    else {
                        triangleIdxFront = i;
                        distanceFront = currentDist;
                        cosineFront = currentCos;
                    }
                }
                else {
                    if (std::abs(currentDist - distanceBack) < EPS_ZFIGHT) {
                        // Z-fighting, lower triangle index wins
                        if (i < triangleIdxFront) {
                            triangleIdxBack = i;
                            distanceBack = currentDist;
                            cosineBack = currentCos;
                        } // else {keep the previous best}
                    }
                    else {
                        triangleIdxBack = i;
                        distanceBack = currentDist;
                        cosineBack = currentCos;
                    }
                }
            }
        }

        // ------------------------ RayBundle methods ------------------------
        
        RayBundle::RayBundle(const Vec3& origin, const std::vector<Vec3>& directions) {
            numRays = directions.size();
            rays.resize(numRays);

            for (int i = 0; i < numRays; ++i) {
                rays.Ox[i] = origin.x;
                rays.Oy[i] = origin.y;
                rays.Oz[i] = origin.z;
                rays.Dx[i] = directions[i].x;
                rays.Dy[i] = directions[i].y;
                rays.Dz[i] = directions[i].z;
            }
#if PLUCKER_KERNEL
            rays.compute_moments();
#endif // end PLUCKER_KERNEL

            radiance = Vec<Real>(numRays, 1);
            totalDistance = Vec<Real>(numRays, 0);
            latestDistance = Vec<Real>(numRays, 0);
            latestCosine = Vec<Real>(numRays, 0);
            latestTriangleIdx = std::vector<int>(numRays, -1);
            previousTriangleIdx = std::vector<int>(numRays, -1);
        }

        RayBundle::RayBundle(const std::vector<Vec3>& origins, const std::vector<Vec3>& directions) {
            numRays = directions.size();
            rays.resize(numRays);

            for (int i = 0; i < numRays; ++i) {
                rays.Ox[i] = origins[i].x;
                rays.Oy[i] = origins[i].y;
                rays.Oz[i] = origins[i].z;
                rays.Dx[i] = directions[i].x;
                rays.Dy[i] = directions[i].y;
                rays.Dz[i] = directions[i].z;
            }
#if PLUCKER_KERNEL
            rays.compute_moments();
#endif // end PLUCKER_KERNEL

            radiance = Vec<Real>(numRays, 1);
            totalDistance = Vec<Real>(numRays, 0);
            latestDistance = Vec<Real>(numRays, 0);
            latestCosine = Vec<Real>(numRays, 0);
            latestTriangleIdx = std::vector<int>(numRays, -1);
            previousTriangleIdx = std::vector<int>(numRays, -1);
        }

        void RayBundle::traceAll(const TriangleMeshSoA& triangles) {
            // Buffers for ray processing
            int triangleIdxFront, triangleIdxBack;
            Real distanceFront, distanceBack, cosineFront, cosineBack;

            for (int i = 0; i < numRays; ++i) {
                // Skip rays that are already invalid.
                if (std::isnan(radiance[i]))
                    continue;

                trace_ray(
                    triangles, rays, i,
                    triangleIdxFront, distanceFront, cosineFront,
                    triangleIdxBack, distanceBack, cosineBack,
                    latestTriangleIdx[i]);

                // NB: Don't mess up the order of operations!
                // Update previousTriangleIdx before overwriting latestTriangleIdx.
                previousTriangleIdx[i] = latestTriangleIdx[i];
                latestTriangleIdx[i] = triangleIdxBack;
                latestDistance[i] = distanceFront;
                latestCosine[i] = cosineFront;
            }
        }

        void RayBundle::advanceAndReflect(const TriangleMeshSoA& triangles) {
            // TODO: Port definition from a different project, if we ever want to trace multiple reflection orders.
#if PLUCKER_KERNEL
            rays.compute_moments();
#endif // end PLUCKER_KERNEL
        }

        void RayBundle::getOrigins(std::vector<Vec3>& origins) const {
            assert(origins.size() == numRays);
            for (int i = 0; i < numRays; ++i) {
                origins[i].x = rays.Ox[i];
                origins[i].y = rays.Oy[i];
                origins[i].z = rays.Oz[i];
            }
        }

        void RayBundle::getDirections(std::vector<Vec3>& directions) const {
            assert(directions.size() == numRays);
            for (int i = 0; i < numRays; ++i) {
                directions[i].x = rays.Dx[i];
                directions[i].y = rays.Dy[i];
                directions[i].z = rays.Dz[i];
            }
        }

        void RayBundle::getTotalDistances(Vec<>& distances) const {
            assert(distances.Rows() == numRays);
            for (int i = 0; i < numRays; ++i) {
                distances[i] = totalDistance[i];
            }
        }

        void RayBundle::getCosines(Vec<>& cosines) const {
            assert(cosines.Rows() == numRays);
            for (int i = 0; i < numRays; ++i) {
                cosines[i] = latestCosine[i];
            }
        }

        void RayBundle::getIndices(std::vector<int>& current, std::vector<int>& previous) const {
            assert(current.size() == numRays);
            assert(previous.size() == numRays);
            for (int i = 0; i < numRays; ++i) {
                current[i] = latestTriangleIdx[i];
                previous[i] = previousTriangleIdx[i];
            }
        }

        void RayBundle::getRadiance(Vec<>& rad) const {
            assert(rad.Rows() == numRays);
            for (int i = 0; i < numRays; ++i) {
                rad[i] = radiance[i];
            }
        }

        // ------------------------ RayPencil methods ------------------------

        RayPencil::RayPencil(int numDirections, bool hemisphereOnly) {
            numRays = numDirections;
            rays.resize(numRays);

            rays.Ox = 0.0;
            rays.Oy = 0.0;
            rays.Oz = 0.0;
            rays.fill_uniform_sphere(hemisphereOnly);
#if PLUCKER_KERNEL
            rays.compute_moments();
#endif // end PLUCKER_KERNEL

            frontDistance = Vec<Real>(numRays, 0);
            backDistance = Vec<Real>(numRays, 0);
            frontCosine = Vec<Real>(numRays, 0);
            backCosine = Vec<Real>(numRays, 0);
            frontTriangleIdx = std::vector<int>(numRays, -1);
            backTriangleIdx = std::vector<int>(numRays, -1);
        }

        RayPencil::RayPencil(const std::vector<Vec3>& directions) {
            numRays = directions.size();
            rays.resize(numRays);

            rays.Ox = 0.0;
            rays.Oy = 0.0;
            rays.Oz = 0.0;
            for (int i = 0; i < numRays; ++i) {
                rays.Dx[i] = directions[i].x;
                rays.Dy[i] = directions[i].y;
                rays.Dz[i] = directions[i].z;
            }
#if PLUCKER_KERNEL
            rays.compute_moments();
#endif // end PLUCKER_KERNEL

            frontDistance = Vec<Real>(numRays, 0);
            backDistance = Vec<Real>(numRays, 0);
            frontCosine = Vec<Real>(numRays, 0);
            backCosine = Vec<Real>(numRays, 0);
            frontTriangleIdx = std::vector<int>(numRays, -1);
            backTriangleIdx = std::vector<int>(numRays, -1);
        }

        void RayPencil::moveOrigin(const Vec3& origin) {
            rays.Ox = origin.x;
            rays.Oy = origin.y;
            rays.Oz = origin.z;
#if PLUCKER_KERNEL
            rays.compute_moments();
#endif // end PLUCKER_KERNEL

            frontDistance = Vec<Real>(numRays, 0);
            backDistance = Vec<Real>(numRays, 0);
            frontCosine = Vec<Real>(numRays, 0);
            backCosine = Vec<Real>(numRays, 0);
            frontTriangleIdx = std::vector<int>(numRays, -1);
            backTriangleIdx = std::vector<int>(numRays, -1);
        }

        void RayPencil::traceAll(const TriangleMeshSoA& triangles) {
            // Buffers for ray processing
            int temp_frontTriangleIdx, temp_backTriangleIdx;
            Real temp_frontDistance, temp_backDistance, temp_frontCosine, temp_backCosine;

            for (int i = 0; i < numRays; ++i) {
                trace_ray(
                    triangles, rays, i,
                    temp_frontTriangleIdx, temp_frontDistance, temp_frontCosine,
                    temp_backTriangleIdx, temp_backDistance, temp_backCosine);

                frontTriangleIdx[i] = temp_frontTriangleIdx;
                frontDistance[i] = temp_frontDistance;
                frontCosine[i] = temp_frontCosine;
                backTriangleIdx[i] = temp_backTriangleIdx;
                backDistance[i] = temp_backDistance;
                backCosine[i] = temp_backCosine;
            }
        }
        
        void RayPencil::clusterDirections(const std::vector<Vec3>& directions,
            std::vector<int>& frontClusters, std::vector<int>& backClusters) const {
            assert(frontClusters.size() == numRays);
            assert(backClusters.size() == numRays);

            // Buffer used while iterating
            std::vector<Real> cosineSimilarity(directions.size(), 0.0);
            int argMin, argMax;

            for (int i = 0; i < numRays; ++i) {
                for (int j = 0; j < directions.size(); ++j) {
                    cosineSimilarity[j] = dot3(
                        rays.Dx[i], rays.Dy[i], rays.Dz[i],
                        directions[j].x, directions[j].y, directions[j].z);
                }
                argMin = std::distance(cosineSimilarity.begin(), std::min_element(cosineSimilarity.begin(), cosineSimilarity.end()));
                argMax = std::distance(cosineSimilarity.begin(), std::max_element(cosineSimilarity.begin(), cosineSimilarity.end()));
                if (cosineSimilarity[argMax] >= -cosineSimilarity[argMin]) {
                    // The highest positive dot product is greater than the absolute value of the lowest negative dot product,
                    // i.e., the forward direction has found a better match than the backward direction.
                    frontClusters[i] = argMax;
                    backClusters[i] = -1;
                } else {
                    // The backward direction has found a better match than the forward direction.
                    frontClusters[i] = -1;
                    backClusters[i] = argMin;
                }
            }
        }

        void RayPencil::getDirections(std::vector<Vec3>& directions) const {
            assert(directions.size() == numRays);
            for (int i = 0; i < numRays; ++i) {
                directions[i].x = rays.Dx[i];
                directions[i].y = rays.Dy[i];
                directions[i].z = rays.Dz[i];
            }
        }

        void RayPencil::getDistances(Vec<>& front, Vec<>& back) const {
            assert(front.Rows() == numRays);
            assert(back.Rows() == numRays);
            for (int i = 0; i < numRays; ++i) {
                front[i] = frontDistance[i];
                back[i] = backDistance[i];
            }
        }

        void RayPencil::getCosines(Vec<>& front, Vec<>& back) const {
            assert(front.Rows() == numRays);
            assert(back.Rows() == numRays);
            for (int i = 0; i < numRays; ++i) {
                front[i] = frontCosine[i];
                back[i] = backCosine[i];
            }
        }

        void RayPencil::getIndices(std::vector<int>& front, std::vector<int>& back) const {
            assert(front.size() == numRays);
            assert(back.size() == numRays);
            for (int i = 0; i < numRays; ++i) {
                front[i] = frontTriangleIdx[i];
                back[i] = backTriangleIdx[i];
            }
        }
	}
}
#pragma optimize ("", on)
