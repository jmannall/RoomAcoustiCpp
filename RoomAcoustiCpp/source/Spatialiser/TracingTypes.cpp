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
        }

        void RayPencilSoA::resize(int n)
        {
            D.resize(n);
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

            // Just to be sure, ensure normalization.
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

            // Just to be sure, ensure normalization.
            normalize_directions();
        }

        void RayBundleSoA::normalize_directions()
        {
            for (int i = 0; i < size(); ++i)
                this->D[i].Normalise();
        }

        void RayPencilSoA::normalize_directions()
        {
            for (int i = 0; i < size(); ++i)
                D[i].Normalise();
        }

        void TriangleMeshSoA::resize(int n)
        {
            A.resize(n);
            edge1.resize(n);
            edge2.resize(n);

            this->n.resize(n);
            patchId.resize(n);
            d0.resize(n);
        }
    }
}
