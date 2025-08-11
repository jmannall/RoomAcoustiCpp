/*
* @brief Defines utilities for basic ray-tracing
*/

#ifndef Tracing_Types_h
#define Tracing_Types_h

// Common headers
#include "Common/Vec.h"
#include "Common/Vec3.h"
#include "Common/Types.h"

/* Controls line-triangle intersection kernel */
#define PLUCKER_KERNEL false
#define LEAN_PLUCKER false

namespace RAC
{
    using namespace Common;
    namespace Spatialiser
    {
        /**
         * @brief Compute a 3D dot product: (ax,ay,az) · (bx,by,bz).
         *
         * @param ax X-component of the first vector.
         * @param ay Y-component of the first vector.
         * @param az Z-component of the first vector.
         * @param bx X-component of the second vector.
         * @param by Y-component of the second vector.
         * @param bz Z-component of the second vector.
         * @return The scalar dot product.
         */
        Real dot3(Real ax, Real ay, Real az, Real bx, Real by, Real bz);

        /**
         * @brief Compute a 3D cross product: (ax,ay,az) × (bx,by,bz).
         *
         * @param ax X-component of the first vector.
         * @param ay Y-component of the first vector.
         * @param az Z-component of the first vector.
         * @param bx X-component of the second vector.
         * @param by Y-component of the second vector.
         * @param bz Z-component of the second vector.
         * @param rx Output X-component of the result.
         * @param ry Output Y-component of the result.
         * @param rz Output Z-component of the result.
         */
        void cross3(Real ax, Real ay, Real az,
            Real bx, Real by, Real bz,
            Real& rx, Real& ry, Real& rz);

        /**
         * @brief Test whether three scalar values have the same sign when treating
         *        values within ±eps as zero (edge-inclusive).
         *
         * A common use is validating the three Plücker side predicates for a triangle.
         *
         * @param s1 First scalar.
         * @param s2 Second scalar.
         * @param s3 Third scalar.
         * @param eps Non-strict zero tolerance.
         * @return true if all three are >= -eps, or all three are <= +eps; otherwise false.
         */
        bool same_sign_with_zero_included(Real s1, Real s2, Real s3, Real eps);

        /**
         * @brief Structure-of-Arrays (SoA) container for a bundle of rays (different origins and directions).
         *
         * For each ray i:
         *  - Origin O = (Ox[i], Oy[i], Oz[i])
         *  - Direction D = (Dx[i], Dy[i], Dz[i])
         *  - #ifdef PLUCKER_KERNEL: Moment   M = O x D = (Mx[i], My[i], Mz[i])  (only used by Plücker tests; precomputed once and reused)
         */
        struct RayBundleSoA {
            /** @name Ray origins */
            ///@{
            std::vector<Real> Ox; /**< X-components of ray origins. */
            std::vector<Real> Oy; /**< Y-components of ray origins. */
            std::vector<Real> Oz; /**< Z-components of ray origins. */
            ///@}

            /** @name Ray directions */
            ///@{
            std::vector<Real> Dx; /**< X-components of ray directions. */
            std::vector<Real> Dy; /**< Y-components of ray directions. */
            std::vector<Real> Dz; /**< Z-components of ray directions. */
            ///@}

#ifdef PLUCKER_KERNEL
            /** @name Ray moments (O x D) */
            ///@{
            std::vector<Real> Mx; /**< X-components of ray moments. */
            std::vector<Real> My; /**< Y-components of ray moments. */
            std::vector<Real> Mz; /**< Z-components of ray moments. */
            ///@}
#endif // end PLUCKER_KERNEL

            /**
             * @brief Number of rays stored.
             * @return Count of rays.
             */
            int size() const { return Dx.size(); }

            /**
             * @brief Resize all internal arrays to hold @p n rays.
             * @param n New number of rays.
             */
            void resize(int n);

            /**
             * @brief Initialize the directions with a uniform sampling of the unit sphere.
             * @param hemisphereOnly If true, generate a uniform sampling of the unit hemisphere (positive Z). Same number of points.
             */
            void fill_uniform_sphere(bool hemisphereOnly);

#ifdef PLUCKER_KERNEL
            /**
             * @brief Compute the ray moment M = O x D for all rays in the bundle.
             *
             * This should be called once after setting origins and directions, and
             * before invoking intersection kernels that use the Plücker predicates.
             */
            void compute_moments();
#endif // end PLUCKER_KERNEL
        };

        /**
         * @brief Structure-of-Arrays (SoA) container for a pencil of rays (same origin and different directions).
         *
         *  - Origin O = (Ox, Oy, Oz)
         * For each ray i:
         *  - Direction D = (Dx[i], Dy[i], Dz[i])
         *  - Moment   M = O x D = (Mx[i], My[i], Mz[i])  (only used by Plücker tests; precomputed once and reused)
         */
        struct RayPencilSoA {
            /** @name Ray origin */
            ///@{
            Real Ox; /**< X-component of ray origin. */
            Real Oy; /**< Y-component of ray origin. */
            Real Oz; /**< Z-component of ray origin. */
            ///@}

            /** @name Ray directions */
            ///@{
            std::vector<Real> Dx; /**< X-components of ray directions. */
            std::vector<Real> Dy; /**< Y-components of ray directions. */
            std::vector<Real> Dz; /**< Z-components of ray directions. */
            ///@}

#ifdef PLUCKER_KERNEL
            /** @name Ray moments (O x D) */
            ///@{
            std::vector<Real> Mx; /**< X-components of ray moments. */
            std::vector<Real> My; /**< Y-components of ray moments. */
            std::vector<Real> Mz; /**< Z-components of ray moments. */
            ///@}
#endif // end PLUCKER_KERNEL

            /**
             * @brief Number of rays stored.
             * @return Count of rays.
             */
            int size() const { return Dx.size(); }

            /**
             * @brief Resize all internal arrays to hold @p n rays.
             * @param n New number of rays.
             */
            void resize(int n);

            /**
             * @brief Initialize the directions with a uniform sampling of the unit sphere.
             * @param hemisphereOnly If true, generate a uniform sampling of the unit hemisphere (positive Z). Same number of points.
             */
            void fill_uniform_sphere(bool hemisphereOnly);

#ifdef PLUCKER_KERNEL
            /**
             * @brief Compute the ray moment M = O x D for all rays in the bundle.
             *
             * This should be called once after setting origins and directions, and
             * before invoking intersection kernels that use the Plücker predicates.
             */
            void compute_moments();
#endif // end PLUCKER_KERNEL
        };

        /**
         * @brief Structure-of-Arrays (SoA) container for triangle data precomputed for line-triangle intersection tests.
         *        either: fat Plücker tests, lean Plücker tests, fat Möller-Trumbore tests.
         */
        struct TriangleMeshSoA {
#ifdef PLUCKER_KERNEL
#ifdef LEAN_PLUCKER
            // TODO: Implement Lean Plücker
#else // not LEAN_PLUCKER
            /** Each triangle stores:
             *  - Edge direction vectors: B−A, C−B, A−C
             *  - Wedge terms (endpoint cross products): AxB, BxC, CxA
             *  - Plane parameters: normal n and constant d0 so that dot(n, X) + d0 = 0
             */

            // ---------------- Edge direction vectors ----------------

            /** @name Edge AB direction (B - A) */
            ///@{
            std::vector<Real> edgeABDirectionX; /**< X-component of edge AB direction. */
            std::vector<Real> edgeABDirectionY; /**< Y-component of edge AB direction. */
            std::vector<Real> edgeABDirectionZ; /**< Z-component of edge AB direction. */
            ///@}

            /** @name Edge BC direction (C - B) */
            ///@{
            std::vector<Real> edgeBCDirectionX; /**< X-component of edge BC direction. */
            std::vector<Real> edgeBCDirectionY; /**< Y-component of edge BC direction. */
            std::vector<Real> edgeBCDirectionZ; /**< Z-component of edge BC direction. */
            ///@}

            /** @name Edge CA direction (A - C) */
            ///@{
            std::vector<Real> edgeCADirectionX; /**< X-component of edge CA direction. */
            std::vector<Real> edgeCADirectionY; /**< Y-component of edge CA direction. */
            std::vector<Real> edgeCADirectionZ; /**< Z-component of edge CA direction. */
            ///@}

            // ---------------- Wedge terms (endpoint cross products) ----------------

            /** @name Wedge for edge AB: A x B */
            ///@{
            std::vector<Real> edgeABWedge_AcrossB_X; /**< X-component of A x B. */
            std::vector<Real> edgeABWedge_AcrossB_Y; /**< Y-component of A x B. */
            std::vector<Real> edgeABWedge_AcrossB_Z; /**< Z-component of A x B. */
            ///@}

            /** @name Wedge for edge BC: B x C */
            ///@{
            std::vector<Real> edgeBCWedge_BcrossC_X; /**< X-component of B x C. */
            std::vector<Real> edgeBCWedge_BcrossC_Y; /**< Y-component of B x C. */
            std::vector<Real> edgeBCWedge_BcrossC_Z; /**< Z-component of B x C. */
            ///@}

            /** @name Wedge for edge CA: C x A */
            ///@{
            std::vector<Real> edgeCAWedge_CcrossA_X; /**< X-component of C x A. */
            std::vector<Real> edgeCAWedge_CcrossA_Y; /**< Y-component of C x A. */
            std::vector<Real> edgeCAWedge_CcrossA_Z; /**< Z-component of C x A. */
            ///@}
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNEL
            // ---------------- Anchor vertex A ----------------
            std::vector<Real> Ax;  /**< X of vertex A. */
            std::vector<Real> Ay;  /**< Y of vertex A. */
            std::vector<Real> Az;  /**< Z of vertex A. */

            // ---------------- Edges from A ----------------
            std::vector<Real> edge1X; /**< X of edge1 = B - A. */
            std::vector<Real> edge1Y; /**< Y of edge1 = B - A. */
            std::vector<Real> edge1Z; /**< Z of edge1 = B - A. */

            std::vector<Real> edge2X; /**< X of edge2 = C - A. */
            std::vector<Real> edge2Y; /**< Y of edge2 = C - A. */
            std::vector<Real> edge2Z; /**< Z of edge2 = C - A. */
#endif // end PLUCKER_KERNEL
            // ---------------- Plane parameters ----------------

            /** @name Triangle plane normal n */
            ///@{
            std::vector<Real> nx; /**< X-component of the triangle normal. */
            std::vector<Real> ny; /**< Y-component of the triangle normal. */
            std::vector<Real> nz; /**< Z-component of the triangle normal. */
            ///@}

            /** @brief Plane constant d0 such that dot(n, X) + d0 = 0. */
            std::vector<Real> d0;

            /**
             * @brief Number of triangles stored.
             * @return Count of triangles.
             */
            int size() const { return d0.size(); }

            /**
             * @brief Resize all internal arrays to hold @p n triangles.
             * @param n New number of triangles.
             */
            void resize(int n);
        };
    }
}

#endif