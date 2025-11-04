#include "Spatialiser/TracingTypes.h"

namespace RAC
{
    using namespace Common;
    namespace Spatialiser
    {
        bool same_sign_with_zero_included(Real s1, Real s2, Real s3, Real eps)
        {
            const bool allNonNeg = (s1 >= -eps) && (s2 >= -eps) && (s3 >= -eps);
            const bool allNonPos = (s1 <= eps) && (s2 <= eps) && (s3 <= eps);
            return allNonNeg || allNonPos;
        }

        void RayBundleSoA::resize(int n)
        {
            O.resize(n);
            D.resize(n);
#if PLUCKER_KERNEL
            M.resize(n);
#endif // end PLUCKER_KERNEL
        }

        void RayPencilSoA::resize(int n)
        {
            D.resize(n);
#if PLUCKER_KERNEL
            M.resize(n);
#endif // end PLUCKER_KERNEL
        }

        // TODO: These two function definitions are identical. I'm sure there's a more elegant way to do that.
        void RayBundleSoA::fill_uniform_sphere(bool hemisphereOnly)
        {
            const int N = hemisphereOnly ? 2 * size() : size();
            const Real Nr = static_cast<Real>(N);
            Real ir, z, r, phi;

            for (int i = 0; i < N; ++i)
            {
                ir = static_cast<Real>(i);

                /* Fibonacci sphere (a.k.a. Vogel sphere): equal-area spacing in z */
                z = 1.0 - 2.0 * (ir + 0.5) / Nr;
                if (hemisphereOnly && z < 0)
                    break; // skip lower hemisphere

                r = std::sqrt(std::max(0.0, 1.0 - z * z));
                phi = PI_2 * std::fmod(ir / PHI, 1.0);

                D[i] = Vec3(r * std::cos(phi), r * std::sin(phi), z);
            }

            // Just to be sure, ensure normalization. Note that this also recomputes the moments if needed.
            normalize_directions();
        }

        void RayPencilSoA::fill_uniform_sphere(bool hemisphereOnly)
        {
            const int N = hemisphereOnly ? 2 * size() : size();
            const Real Nr = static_cast<Real>(N);
            Real ir, z, r, phi;

            for (int i = 0; i < N; ++i)
            {
                ir = static_cast<Real>(i);

                /* Fibonacci sphere (a.k.a. Vogel sphere): equal-area spacing in z */
                z = 1.0 - 2.0 * (ir + 0.5) / Nr;
                if (hemisphereOnly && z < 0)
                    break; // skip lower hemisphere

                r = std::sqrt(std::max(0.0, 1.0 - z * z));
                phi = PI_2 * std::fmod(ir / PHI, 1.0);

                D[i] = Vec3(r * std::cos(phi), r * std::sin(phi), z);
            }

            // Just to be sure, ensure normalization. Note that this also recomputes the moments if needed.
            normalize_directions();
        }

        void RayBundleSoA::normalize_directions()
        {
            for (int i = 0; i < size(); ++i)
                this->D[i].Normalise();
#if PLUCKER_KERNEL
            compute_moments();
#endif
        }

        void RayPencilSoA::normalize_directions()
        {
            for (int i = 0; i < size(); ++i)
                D[i].Normalise();
#if PLUCKER_KERNEL
            compute_moments();
#endif
        }

#if PLUCKER_KERNEL
        void RayBundleSoA::compute_moments()
        {
            for (int i = 0; i < size(); ++i)
            {
				M[i] = O[i].cross(D[i]);
            }
        }

        void RayPencilSoA::compute_moments()
        {
            for (int i = 0; i < size(); ++i)
            {
                M[i] = O.cross(D[i]);
            }
        }
#endif // end PLUCKER_KERNEL

        void TriangleMeshSoA::resize(int n)
        {
#if PLUCKER_KERNEL
#if LEAN_PLUCKER
            // TODO: Implement Lean Plücker
#else // not LEAN_PLUCKER
            edgeABDirection.resize(n);
            edgeBCDirection.resize(n);
            edgeCADirection.resize(n);

            edgeABWedge_AcrossB.resize(n);
            edgeBCWedge_BcrossC.resize(n);
            edgeCAWedge_CcrossA.resize(n);
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNEL
            A.resize(n);
            edge1.resize(n);
            edge2.resize(n);
#endif // end PLUCKER_KERNEL
            this->n.resize(n);
            patchId.resize(n);
            d0.resize(n);
        }
    }
}
