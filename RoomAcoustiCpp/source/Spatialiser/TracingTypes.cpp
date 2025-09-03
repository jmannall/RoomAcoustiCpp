#include "Spatialiser/TracingTypes.h"

#if defined _DEBUG || defined DEBUG_RTM
#pragma optimize("", off)
#endif

namespace RAC
{
    using namespace Common;
    namespace Spatialiser
    {
        Real norm3(Real x, Real y, Real z)
        {
            return std::sqrt(x * x + y * y + z * z);
        }

        Real dot3(Real ax, Real ay, Real az, Real bx, Real by, Real bz)
        {
            return ax * bx + ay * by + az * bz;
        }

        void cross3(Real ax, Real ay, Real az,
            Real bx, Real by, Real bz,
            Real& rx, Real& ry, Real& rz) {
            rx = ay * bz - az * by;
            ry = az * bx - ax * bz;
            rz = ax * by - ay * bx;
        }

        bool same_sign_with_zero_included(Real s1, Real s2, Real s3, Real eps)
        {
            const bool allNonNeg = (s1 >= -eps) && (s2 >= -eps) && (s3 >= -eps);
            const bool allNonPos = (s1 <= eps) && (s2 <= eps) && (s3 <= eps);
            return allNonNeg || allNonPos;
        }

        void RayBundleSoA::resize(int n)
        {
            Ox.resize(n);
            Oy.resize(n);
            Oz.resize(n);
            Dx.resize(n);
            Dy.resize(n);
            Dz.resize(n);
#if PLUCKER_KERNEL
            Mx.resize(n);
            My.resize(n);
            Mz.resize(n);
#endif // end PLUCKER_KERNEL
        }

        void RayPencilSoA::resize(int n)
        {
            Dx.resize(n);
            Dy.resize(n);
            Dz.resize(n);
#if PLUCKER_KERNEL
            Mx.resize(n);
            My.resize(n);
            Mz.resize(n);
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

                Dx[i] = r * std::cos(phi);
                Dy[i] = r * std::sin(phi);
                Dz[i] = z;
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

                Dx[i] = r * std::cos(phi);
                Dy[i] = r * std::sin(phi);
                Dz[i] = z;
            }

            // Just to be sure, ensure normalization. Note that this also recomputes the moments if needed.
            normalize_directions();
        }

        void RayBundleSoA::normalize_directions()
        {
            Real norm;
            for (int i = 0; i < size(); ++i)
            {
                norm = norm3(Dx[i], Dy[i], Dz[i]);
                if (norm == 0) continue;
                if (norm == 1) continue;
                Dx[i] /= norm;
                Dy[i] /= norm;
                Dz[i] /= norm;
            }
#if PLUCKER_KERNEL
            compute_moments();
#endif
        }

        void RayPencilSoA::normalize_directions()
        {
            Real norm;
            for (int i = 0; i < size(); ++i)
            {
                norm = norm3(Dx[i], Dy[i], Dz[i]);
                if (norm == 0) continue;
                if (norm == 1) continue;
                Dx[i] /= norm;
                Dy[i] /= norm;
                Dz[i] /= norm;
            }
#if PLUCKER_KERNEL
            compute_moments();
#endif
        }

#if PLUCKER_KERNEL
        void RayBundleSoA::compute_moments()
        {
            for (int i = 0; i < size(); ++i)
            {
                cross3(Ox[i], Oy[i], Oz[i],
                    Dx[i], Dy[i], Dz[i],
                    Mx[i], My[i], Mz[i]);
            }
        }

        void RayPencilSoA::compute_moments()
        {
            for (int i = 0; i < size(); ++i)
            {
                cross3(Ox, Oy, Oz,
                    Dx[i], Dy[i], Dz[i],
                    Mx[i], My[i], Mz[i]);
            }
        }
#endif // end PLUCKER_KERNEL

        void TriangleMeshSoA::resize(int n)
        {
#if PLUCKER_KERNEL
#if LEAN_PLUCKER
            // TODO: Implement Lean Plücker
#else // not LEAN_PLUCKER
            edgeABDirectionX.resize(n);
            edgeABDirectionY.resize(n);
            edgeABDirectionZ.resize(n);
            edgeBCDirectionX.resize(n);
            edgeBCDirectionY.resize(n);
            edgeBCDirectionZ.resize(n);
            edgeCADirectionX.resize(n);
            edgeCADirectionY.resize(n);
            edgeCADirectionZ.resize(n);

            edgeABWedge_AcrossB_X.resize(n);
            edgeABWedge_AcrossB_Y.resize(n);
            edgeABWedge_AcrossB_Z.resize(n);
            edgeBCWedge_BcrossC_X.resize(n);
            edgeBCWedge_BcrossC_Y.resize(n);
            edgeBCWedge_BcrossC_Z.resize(n);
            edgeCAWedge_CcrossA_X.resize(n);
            edgeCAWedge_CcrossA_Y.resize(n);
            edgeCAWedge_CcrossA_Z.resize(n);
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNEL
            Ax.resize(n);
            Ay.resize(n);
            Az.resize(n);
            edge1X.resize(n);
            edge1Y.resize(n);
            edge1Z.resize(n);
            edge2X.resize(n);
            edge2Y.resize(n);
            edge2Z.resize(n);
#endif // end PLUCKER_KERNEL
            nx.resize(n);
            ny.resize(n);
            nz.resize(n);
            nodeID.resize(n);
            d0.resize(n);
        }
    }
}

#if defined _DEBUG || defined DEBUG_RTM
#pragma optimize("", on)
#endif
