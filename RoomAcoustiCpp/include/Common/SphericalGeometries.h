/*
* @brief Functions for generating spherically distributed points
*
*/

#ifndef Common_SphericalGeometries_h
#define Common_SphericalGeometries_h

// C++ headers
#include <vector>

// Common headers
#include "Common/Types.h"
#include "Common/Vec3.h"

namespace RAC
{
	namespace Common
	{
		inline void Tetrahedron(std::vector<Vec3>& vertices, bool one)
		{
			if (one)
			{
				vertices.emplace_back(REAL_CONST(1.0), REAL_CONST(1.0), REAL_CONST(1.0));
				vertices.emplace_back(REAL_CONST(1.0), REAL_CONST(-1.0), REAL_CONST(-1.0));
				vertices.emplace_back(REAL_CONST(-1.0), REAL_CONST(1.0), REAL_CONST(-1.0));
				vertices.emplace_back(REAL_CONST(-1.0), REAL_CONST(-1.0), REAL_CONST(1.0));
			}
			else
			{
				vertices.emplace_back(REAL_CONST(-1.0), REAL_CONST(-1.0), REAL_CONST(-1.0));
				vertices.emplace_back(REAL_CONST(-1.0), REAL_CONST(1.0), REAL_CONST(1.0));
				vertices.emplace_back(REAL_CONST(1.0), REAL_CONST(-1.0), REAL_CONST(1.0));
				vertices.emplace_back(REAL_CONST(1.0), REAL_CONST(1.0), REAL_CONST(-1.0));
			}
		}

		inline void Octahedron(std::vector<Vec3>& vertices)
		{
			vertices.emplace_back(REAL_CONST(1.0), REAL_CONST(0.0), REAL_CONST(0.0));
			vertices.emplace_back(REAL_CONST(0.0), REAL_CONST(1.0), REAL_CONST(0.0));
			vertices.emplace_back(REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(1.0));

			vertices.emplace_back(REAL_CONST(-1.0), REAL_CONST(0.0), REAL_CONST(0.0));
			vertices.emplace_back(REAL_CONST(0.0), REAL_CONST(-1.0), REAL_CONST(0.0));
			vertices.emplace_back(REAL_CONST(0.0), REAL_CONST(0.0), REAL_CONST(-1.0));
		}

		inline void Cube(std::vector<Vec3>& vertices)
		{
			vertices.emplace_back(REAL_CONST(1.0), REAL_CONST(1.0), REAL_CONST(1.0));
			vertices.emplace_back(REAL_CONST(1.0), REAL_CONST(-1.0), REAL_CONST(1.0));
			vertices.emplace_back(REAL_CONST(1.0), REAL_CONST(1.0), REAL_CONST(-1.0));
			vertices.emplace_back(REAL_CONST(1.0), REAL_CONST(-1.0), REAL_CONST(-1.0));

			vertices.emplace_back(REAL_CONST(-1.0), REAL_CONST(-1.0), REAL_CONST(-1.0));
			vertices.emplace_back(REAL_CONST(-1.0), REAL_CONST(1.0), REAL_CONST(-1.0));
			vertices.emplace_back(REAL_CONST(-1.0), REAL_CONST(-1.0), REAL_CONST(1.0));
			vertices.emplace_back(REAL_CONST(-1.0), REAL_CONST(1.0), REAL_CONST(1.0));
		}

		inline void Icosahedron(std::vector<Vec3>& vertices, bool one)
		{
			Real phi = (REAL_CONST(1.0) + sqrt(REAL_CONST(5.0))) / REAL_CONST(2.0);

			if (one)
			{
				vertices.emplace_back(REAL_CONST(0.0), phi, REAL_CONST(1.0));
				vertices.emplace_back(phi, REAL_CONST(1.0), REAL_CONST(0.0));
				vertices.emplace_back(REAL_CONST(1.0), REAL_CONST(0.0), phi);

				vertices.emplace_back(REAL_CONST(0.0), phi, REAL_CONST(-1.0));
				vertices.emplace_back(phi, REAL_CONST(-1.0), REAL_CONST(0.0));
				vertices.emplace_back(REAL_CONST(-1.0), REAL_CONST(0.0), phi);

				vertices.emplace_back(REAL_CONST(0.0), -phi, REAL_CONST(-1.0));
				vertices.emplace_back(-phi, REAL_CONST(-1.0), REAL_CONST(0.0));
				vertices.emplace_back(REAL_CONST(-1.0), REAL_CONST(0.0), -phi);

				vertices.emplace_back(REAL_CONST(0.0), -phi, REAL_CONST(1.0));
				vertices.emplace_back(-phi, REAL_CONST(1.0), REAL_CONST(0.0));
				vertices.emplace_back(REAL_CONST(1.0), REAL_CONST(0.0), -phi);
			}
			else
			{
				vertices.emplace_back(REAL_CONST(0.0), REAL_CONST(1.0), phi);
				vertices.emplace_back(REAL_CONST(1.0), phi, REAL_CONST(0.0));
				vertices.emplace_back(phi, REAL_CONST(0.0), REAL_CONST(1.0));

				vertices.emplace_back(REAL_CONST(0.0), REAL_CONST(-1.0), phi);
				vertices.emplace_back(REAL_CONST(-1.0), phi, REAL_CONST(0.0));
				vertices.emplace_back(phi, REAL_CONST(0.0), REAL_CONST(-1.0));

				vertices.emplace_back(REAL_CONST(0.0), REAL_CONST(1.0), -phi);
				vertices.emplace_back(REAL_CONST(1.0), -phi, REAL_CONST(0.0));
				vertices.emplace_back(-phi, REAL_CONST(0.0), REAL_CONST(1.0));

				vertices.emplace_back(REAL_CONST(0.0), REAL_CONST(1.0), -phi);
				vertices.emplace_back(REAL_CONST(1.0), -phi, REAL_CONST(0.0));
				vertices.emplace_back(-phi, REAL_CONST(0.0), REAL_CONST(1.0));
			}
		}

		inline void Dodecahedron(std::vector<Vec3>& vertices, bool one)
		{
			Real phi = (REAL_CONST(1.0) + sqrt(REAL_CONST(5.0))) / REAL_CONST(2.0);
			Real invphi = REAL_CONST(1.0) / phi;

			Cube(vertices);
			if (one)
			{
				vertices.emplace_back(REAL_CONST(0.0), invphi, phi);
				vertices.emplace_back(invphi, phi, REAL_CONST(0.0));
				vertices.emplace_back(phi, REAL_CONST(0.0), invphi);

				vertices.emplace_back(REAL_CONST(0.0), -invphi, phi);
				vertices.emplace_back(-invphi, phi, REAL_CONST(0.0));
				vertices.emplace_back(phi, REAL_CONST(0.0), -invphi);

				vertices.emplace_back(REAL_CONST(0.0), -invphi, -phi);
				vertices.emplace_back(-invphi, -phi, REAL_CONST(0.0));
				vertices.emplace_back(-phi, REAL_CONST(0.0), -invphi);

				vertices.emplace_back(REAL_CONST(0.0), invphi, -phi);
				vertices.emplace_back(invphi, -phi, REAL_CONST(0.0));
				vertices.emplace_back(-phi, REAL_CONST(0.0), invphi);
			}
			else
			{
				vertices.emplace_back(REAL_CONST(0.0), phi, invphi);
				vertices.emplace_back(phi, invphi, REAL_CONST(0.0));
				vertices.emplace_back(invphi, REAL_CONST(0.0), phi);

				vertices.emplace_back(REAL_CONST(0.0), phi, -invphi);
				vertices.emplace_back(phi, -invphi, REAL_CONST(0.0));
				vertices.emplace_back(-invphi, REAL_CONST(0.0), phi);

				vertices.emplace_back(REAL_CONST(0.0), -phi, -invphi);
				vertices.emplace_back(-phi, -invphi, REAL_CONST(0.0));
				vertices.emplace_back(-invphi, REAL_CONST(0.0), -phi);

				vertices.emplace_back(REAL_CONST(0.0), -phi, invphi);
				vertices.emplace_back(-phi, invphi, REAL_CONST(0.0));
				vertices.emplace_back(invphi, REAL_CONST(0.0), -phi);
			}
		}
	}
}

#endif