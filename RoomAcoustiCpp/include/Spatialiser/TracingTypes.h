/*
* @brief Defines utilities for basic ray-tracing
*/

#ifndef Tracing_Types_h
#define Tracing_Types_h

// Common headers
#include "Common/Vec.h"
#include "Common/Vec3.h"
#include "Common/Types.h"
#include "Common/Definitions.h"

/* Controls line-triangle intersection kernel */
#define PLUCKER_KERNEL false
#define LEAN_PLUCKER false
#define SELF_SHADOWING_RADIUS 0.0

namespace RAC
{
    using namespace Common;
    namespace Spatialiser
    {
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
         *  - #if PLUCKER_KERNEL: Moment   M = O x D = (Mx[i], My[i], Mz[i])  (only used by Plücker tests; precomputed once and reused)
         */
        struct RayBundleSoA {
            /** @name Ray origins */
            ///@{
            std::vector<Vec3> O ; /**< ray origins. */
            ///@}

            /** @name Ray directions */
            ///@{
            std::vector<Vec3> D; /**< ray directions. */
            ///@}

#if PLUCKER_KERNEL
            /** @name Ray moments (O x D) */
            ///@{
            std::vector<Vec3> M; /**< components of ray moments. */
            ///@}
#endif // end PLUCKER_KERNEL

            /**
             * @brief Number of rays stored.
             * @return Count of rays.
             */
            inline int size() const { return ToInt( D.size() ); }

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

            /**
             * @brief Normalize all direction vectors.
             */
            void normalize_directions();

#if PLUCKER_KERNEL
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
            Vec3 O;  /**< ray origin. */
            ///@}

            /** @name Ray directions */
            ///@{
            std::vector<Vec3> D; /**< ray directions. */
            ///@}

#if PLUCKER_KERNEL
            /** @name Ray moments (O x D) */
            ///@{
            std::vector<Vec3> M; /**< ray moments. */
            ///@}
#endif // end PLUCKER_KERNEL

            /**
             * @brief Number of rays stored.
             * @return Count of rays.
             */
            inline int size() const { return ToInt( D.size() ); }

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

            /**
             * @brief Normalize all direction vectors.
             */
            void normalize_directions();

#if PLUCKER_KERNEL
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
#if PLUCKER_KERNEL
#if LEAN_PLUCKER
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
            std::vector<Vec3> edgeABDirection; /**< edge AB direction. */
            ///@}

            /** @name Edge BC direction (C - B) */
            ///@{
            std::vector<Vec3> edgeBCDirection; /**< edge BC direction. */
            ///@}

            /** @name Edge CA direction (A - C) */
            ///@{
            std::vector<Vec3> edgeCADirection; /**< edge CA direction. */
            ///@}

            // ---------------- Wedge terms (endpoint cross products) ----------------

            /** @name Wedge for edge AB: A x B */
            ///@{
            std::vector<Vec3> edgeABWedge_AcrossB; /**< A x B. */
            ///@}

            /** @name Wedge for edge BC: B x C */
            ///@{
            std::vector<Vec3> edgeBCWedge_BcrossC; /**< B x C. */
            ///@}

            /** @name Wedge for edge CA: C x A */
            ///@{
            std::vector<Vec3> edgeCAWedge_CcrossA; /**< C x A. */
            ///@}
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNEL
            // ---------------- Anchor vertex A ----------------
            std::vector<Vec3> A;  /**< vertex A. */

            // ---------------- Edges from A ----------------
            std::vector<Vec3> edge1; /**< edge1 = B - A. */

            std::vector<Vec3> edge2; /**< edge2 = C - A. */
#endif // end PLUCKER_KERNEL
            // ---------------- Plane parameters ----------------

            /** @name Triangle plane normal n */
            ///@{
            std::vector<Vec3> n; /**< X-component of the triangle normal. */
            ///@}

			/** @brief Node that the triangle belongs to */
            std::vector<int> patchId;

            /** @brief Plane constant d0 such that dot(n, X) + d0 = 0. */
            std::vector<Real> d0;

            /**
             * @brief Number of triangles stored.
             * @return Count of triangles.
             */
            inline int size() const { return ToInt( d0.size() ); }

            /**
             * @brief Resize all internal arrays to hold @p n triangles.
             * @param n New number of triangles.
             */
            void resize(int n);
        };
    }
}

#endif