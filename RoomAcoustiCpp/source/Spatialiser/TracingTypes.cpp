#include "Spatialiser/TracingTypes.h"

namespace RAC
{
    namespace Common
    {
        Real dot3(Real ax, Real ay, Real az, Real bx, Real by, Real bz) {
            return ax * bx + ay * by + az * bz;
        }

        void cross3(Real ax, Real ay, Real az,
            Real bx, Real by, Real bz,
            Real& rx, Real& ry, Real& rz) {
            rx = ay * bz - az * by;
            ry = az * bx - ax * bz;
            rz = ax * by - ay * bx;
        }

        bool same_sign_with_zero_included(Real s1, Real s2, Real s3, Real eps) {
            const bool allNonNeg = (s1 >= -eps) && (s2 >= -eps) && (s3 >= -eps);
            const bool allNonPos = (s1 <= eps) && (s2 <= eps) && (s3 <= eps);
            return allNonNeg || allNonPos;
        }

        void RayBundleSoA::resize(int n) {
            Ox.resize(n);
            Oy.resize(n);
            Oz.resize(n);
            Dx.resize(n);
            Dy.resize(n);
            Dz.resize(n);
#ifdef PLUCKER_KERNEL
            Mx.resize(n);
            My.resize(n);
            Mz.resize(n);
#endif // end PLUCKER_KERNEL
        }

        void RayPencilSoA::resize(int n) {
            Dx.resize(n);
            Dy.resize(n);
            Dz.resize(n);
#ifdef PLUCKER_KERNEL
            Mx.resize(n);
            My.resize(n);
            Mz.resize(n);
#endif // end PLUCKER_KERNEL
        }

#ifdef PLUCKER_KERNEL
        void RayBundleSoA::compute_moments() {
            const int n = size();
            for (int i = 0; i < n; ++i) {
                cross3(Ox[i], Oy[i], Oz[i],
                    Dx[i], Dy[i], Dz[i],
                    Mx[i], My[i], Mz[i]);
            }
        }

        void RayPencilSoA::compute_moments() {
            const int n = size();
            for (int i = 0; i < n; ++i) {
                cross3(Ox, Oy, Oz,
                    Dx[i], Dy[i], Dz[i],
                    Mx[i], My[i], Mz[i]);
            }
        }
#endif // end PLUCKER_KERNEL

        void TriangleMeshSoA::resize(int n) {
#ifdef PLUCKER_KERNEL
#ifdef LEAN_PLUCKER
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
            d0.resize(n);
        }

        TriangleMeshSoA build_mesh_soa_from_vertices(
            const std::vector<Real>& Ax, const std::vector<Real>& Ay, const std::vector<Real>& Az,
            const std::vector<Real>& Bx, const std::vector<Real>& By, const std::vector<Real>& Bz,
            const std::vector<Real>& Cx, const std::vector<Real>& Cy, const std::vector<Real>& Cz) {

            const int triangleCount = Ax.size();
            assert(Ay.size() == triangleCount && Az.size() == triangleCount);
            assert(Bx.size() == triangleCount && By.size() == triangleCount && Bz.size() == triangleCount);
            assert(Cx.size() == triangleCount && Cy.size() == triangleCount && Cz.size() == triangleCount);

            TriangleMeshSoA out;
            out.resize(triangleCount);

            for (int i = 0; i < triangleCount; ++i) {
#ifdef PLUCKER_KERNEL
#ifdef LEAN_PLUCKER
                // TODO: Implement Lean Plücker
#else // not LEAN_PLUCKER
                // ----- Edge direction vectors -----
                // Edge AB = B - A
                out.edgeABDirectionX[i] = Bx[i] - Ax[i];
                out.edgeABDirectionY[i] = By[i] - Ay[i];
                out.edgeABDirectionZ[i] = Bz[i] - Az[i];

                // Edge BC = C - B
                out.edgeBCDirectionX[i] = Cx[i] - Bx[i];
                out.edgeBCDirectionY[i] = Cy[i] - By[i];
                out.edgeBCDirectionZ[i] = Cz[i] - Bz[i];

                // Edge CA = A - C
                out.edgeCADirectionX[i] = Ax[i] - Cx[i];
                out.edgeCADirectionY[i] = Ay[i] - Cy[i];
                out.edgeCADirectionZ[i] = Az[i] - Cz[i];

                // ----- Wedge terms (endpoint cross products) -----
                // These are used by Plücker side predicates: s = dot(D, (PxQ)) + dot(M, (Q-P)).
                Real wedgeX, wedgeY, wedgeZ;

                // A x B
                cross3(
                    Ax[i], Ay[i], Az[i],
                    Bx[i], By[i], Bz[i],
                    wedgeX, wedgeY, wedgeZ);
                out.edgeABWedge_AcrossB_X[i] = wedgeX;
                out.edgeABWedge_AcrossB_Y[i] = wedgeY;
                out.edgeABWedge_AcrossB_Z[i] = wedgeZ;

                // B x C
                cross3(
                    Bx[i], By[i], Bz[i],
                    Cx[i], Cy[i], Cz[i],
                    wedgeX, wedgeY, wedgeZ);
                out.edgeBCWedge_BcrossC_X[i] = wedgeX;
                out.edgeBCWedge_BcrossC_Y[i] = wedgeY;
                out.edgeBCWedge_BcrossC_Z[i] = wedgeZ;

                // C x A
                cross3(
                    Cx[i], Cy[i], Cz[i],
                    Ax[i], Ay[i], Az[i],
                    wedgeX, wedgeY, wedgeZ);
                out.edgeCAWedge_CcrossA_X[i] = wedgeX;
                out.edgeCAWedge_CcrossA_Y[i] = wedgeY;
                out.edgeCAWedge_CcrossA_Z[i] = wedgeZ;
#endif // end LEAN_PLUCKER
#else // not PLUCKER_KERNEL
                // ----- Anchor vertex A -----
                out.Ax[i] = Ax[i];
                out.Ay[i] = Ay[i];
                out.Az[i] = Az[i];

                // ----- Edges from A -----
                out.edge1X[i] = Bx[i] - Ax[i];
                out.edge1Y[i] = By[i] - Ay[i];
                out.edge1Z[i] = Bz[i] - Az[i];
                out.edge2X[i] = Cx[i] - Ax[i];
                out.edge2Y[i] = Cy[i] - Ay[i];
                out.edge2Z[i] = Cz[i] - Az[i];
#endif // end PLUCKER_KERNEL

                // ----- Plane parameters: normal n and plane constant d0 -----
                // Compute two edges from A for the normal: e1 = B - A, e2 = C - A.
                // n = e1 x e2 (right-handed cross product). Unnormalized on purpose.
                Real nx, ny, nz;
                cross3(
                    Bx[i] - Ax[i], By[i] - Ay[i], Bz[i] - Az[i],
                    Cx[i] - Ax[i], Cy[i] - Ay[i], Cz[i] - Az[i],
                    nx, ny, nz);
                out.nx[i] = nx;
                out.ny[i] = ny;
                out.nz[i] = nz;

                // Plane constant d0 such that: dot(n, X) + d0 = 0.
                // Using point A on the plane: d0 = -dot(n, A).
                out.d0[i] = -(nx * Ax[i] + ny * Ay[i] + nz * Az[i]);
            }

            return out;
        }
    }
}