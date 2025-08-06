#include "Spatialiser/TracingUtils.h"

namespace RAC
{
	namespace Common
	{
        // ------------------------ Intersection kernels ------------------------
        
        void intersection_test(
            TriangleMeshSoA& triangles, int triangleIndex,
            RayBundleSoA& rays, int rayIndex,
            Real& distance, Real& cosine) {
#ifdef PLUCKER_KERNEL
#ifdef LEAN_PLUCKER
            // TODO: Implement Lean Plücker
#else // not LEAN_PLUCKER
            // TODO: Write definition
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNEL
            // TODO: Write definition
#endif // end PLUCKER_KERNEL
        }

        void intersection_test(
            TriangleMeshSoA& triangles, int triangleIndex,
            RayPencilSoA& rays, int rayIndex,
            Real& distance, Real& cosine) {
#ifdef PLUCKER_KERNEL
#ifdef LEAN_PLUCKER
            // TODO: Implement Lean Plücker
#else // not LEAN_PLUCKER
            // TODO: Write definition
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNEL
            // TODO: Write definition
#endif // end PLUCKER_KERNEL
        }

        void intersection_test(
            TriangleMeshSoA& triangles, int triangleIndex,
            Vec3& rayOrigin, Vec3& rayDirection, Vec3& rayMoment,
            Real& distance, Real& cosine) {
#ifdef PLUCKER_KERNEL
#ifdef LEAN_PLUCKER
            // TODO: Write definition
#else // not LEAN_PLUCKER
            // TODO: Implement Lean Plücker
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNEL
            // TODO: Write definition
#endif // end PLUCKER_KERNEL
        }

        // ------------------------ Tracing loop kernels ------------------------

        void trace_ray(
            TriangleMeshSoA& triangles, RayBundleSoA& rays, int rayIndex,
            int& triangleIdxFront, Real& distanceFront, Real& cosineFront,
            int& triangleIdxBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex) {
            // TODO: Write definition
        }

        void trace_ray(
            TriangleMeshSoA& triangles, RayPencilSoA& rays, int rayIndex,
            int& triangleIdxFront, Real& distanceFront, Real& cosineFront,
            int& triangleIdxBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex) {
            // TODO: Write definition
        }

        void trace_ray(
            TriangleMeshSoA& triangles, Vec3& rayOrigin, Vec3& rayDirection, Vec3& rayMoment,
            int& triangleIdxFront, Real& distanceFront, Real& cosineFront,
            int& triangleIdxBack, Real& distanceBack, Real& cosineBack,
            int ignoredTriangleIndex) {
            // TODO: Write definition
        }

        // ------------------------ RayBundle methods ------------------------
        
        RayBundle::RayBundle(Vec3& origin, Matrix<>& directions) {
            // TODO: Write definition
#ifdef PLUCKER_KERNEL
            // Compute moments
            // TODO: Write definition
#endif // end PLUCKER_KERNEL
        }

        RayBundle::RayBundle(Matrix<>& origins, Matrix<>& directions) {
            // TODO: Write definition
#ifdef PLUCKER_KERNEL
            // Compute moments
            // TODO: Write definition
#endif // end PLUCKER_KERNEL
        }

        void RayBundle::trace_all() {
            // TODO: Write definition
        }

        void RayBundle::advance() {
            // TODO: Write definition
#ifdef PLUCKER_KERNEL
            // Compute moments
            // TODO: Write definition
#endif // end PLUCKER_KERNEL
        }

        void RayBundle::get_total_distances(Vec<>& totalDistances) {
            // TODO: Write definition
        }

        void RayBundle::get_indices(std::vector<int>& currentIndices, std::vector<int>& previousIndices) {
            // TODO: Write definition
        }

        Vec<Real> RayBundle::get_radiance() {
            // TODO: Write definition
        }

        RayPencil::RayPencil(Vec3& origin, int numDirections, bool hemisphereOnly) {
            // TODO: Write definition
#ifdef PLUCKER_KERNEL
            // Compute moments
            // TODO: Write definition
#endif // end PLUCKER_KERNEL
        }

        // ------------------------ RayPencil methods ------------------------

        RayPencil::RayPencil(Vec3& origin, Matrix<>& directions) {
            // TODO: Write definition
#ifdef PLUCKER_KERNEL
            // Compute moments
            // TODO: Write definition
#endif // end PLUCKER_KERNEL
        }

        void RayPencil::move_origin(Vec3& origin) {
            // TODO: Write definition
#ifdef PLUCKER_KERNEL
            // Compute moments
            // TODO: Write definition
#endif // end PLUCKER_KERNEL
        }

        void RayPencil::trace_all() {
            // TODO: Write definition
        }

        void RayPencil::get_distances(Vec<>& frontDistances, Vec<>& backDistances) {
            // TODO: Write definition
        }

        void RayPencil::get_cosines(Vec<>& frontCosines, Vec<>& backCosines) {
            // TODO: Write definition
        }

        void RayPencil::get_indices(std::vector<int>& frontIndices, std::vector<int>& backIndices) {
            // TODO: Write definition
        }
	}
}