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

namespace RAC
{
    using namespace Common;
    namespace Spatialiser
    {
        /**
         * @brief Test whether three scalar values have the same sign when treating
         *        values within ±eps as zero (edge-inclusive).
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
        };

        /**
         * @brief Structure-of-Arrays (SoA) container for a pencil of rays (shared origin but different directions).
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
        };

        /**
         * @brief Structure-of-Arrays (SoA) container for triangle data precomputed for Möller-Trumbore line-triangle intersection tests.
         */
        struct TriangleMeshSoA {
            // ---------------- Anchor vertex A ----------------
            std::vector<Vec3> A;  /**< vertex A. */

            // ---------------- Edges from A ----------------
            std::vector<Vec3> edge1; /**< edge1 = B - A. */

            std::vector<Vec3> edge2; /**< edge2 = C - A. */

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