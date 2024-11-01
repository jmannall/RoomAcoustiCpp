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
				vertices.push_back(Vec3(1.0, 1.0, 1.0));
				vertices.push_back(Vec3(1.0, -1.0, -1.0));
				vertices.push_back(Vec3(-1.0, 1.0, -1.0));
				vertices.push_back(Vec3(-1.0, -1.0, 1.0));
			}
			else
			{
				vertices.push_back(Vec3(-1.0, -1.0, -1.0));
				vertices.push_back(Vec3(-1.0, 1.0, 1.0));
				vertices.push_back(Vec3(1.0, -1.0, 1.0));
				vertices.push_back(Vec3(1.0, 1.0, -1.0));
			}
		}

		inline void Octahedron(std::vector<Vec3>& vertices)
		{
			vertices.push_back(Vec3(1.0, 0.0, 0.0));
			vertices.push_back(Vec3(0.0, 1.0, 0.0));
			vertices.push_back(Vec3(0.0, 0.0, 1.0));

			vertices.push_back(Vec3(-1.0, 0.0, 0.0));
			vertices.push_back(Vec3(0.0, -1.0, 0.0));
			vertices.push_back(Vec3(0.0, 0.0, -1.0));
		}

		inline void Cube(std::vector<Vec3>& vertices)
		{
			vertices.push_back(Vec3(1.0, 1.0, 1.0));
			vertices.push_back(Vec3(1.0, -1.0, 1.0));
			vertices.push_back(Vec3(1.0, 1.0, -1.0));
			vertices.push_back(Vec3(1.0, -1.0, -1.0));

			vertices.push_back(Vec3(-1.0, -1.0, -1.0));
			vertices.push_back(Vec3(-1.0, 1.0, -1.0));
			vertices.push_back(Vec3(-1.0, -1.0, 1.0));
			vertices.push_back(Vec3(-1.0, 1.0, 1.0));
		}

		inline void Icosahedron(std::vector<Vec3>& vertices, bool one)
		{
			Real phi = (1.0 + sqrt(5.0)) / 2.0;

			if (one)
			{
				vertices.push_back(Vec3(0.0, phi, 1.0));
				vertices.push_back(Vec3(phi, 1.0, 0.0));
				vertices.push_back(Vec3(1.0, 0.0, phi));

				vertices.push_back(Vec3(0.0, phi, -1.0));
				vertices.push_back(Vec3(phi, -1.0, 0.0));
				vertices.push_back(Vec3(-1.0, 0.0, phi));

				vertices.push_back(Vec3(0.0, -phi, -1.0));
				vertices.push_back(Vec3(-phi, -1.0, 0.0));
				vertices.push_back(Vec3(-1.0, 0.0, -phi));

				vertices.push_back(Vec3(0.0, -phi, 1.0));
				vertices.push_back(Vec3(-phi, 1.0, 0.0));
				vertices.push_back(Vec3(1.0, 0.0, -phi));
			}
			else
			{
				vertices.push_back(Vec3(0.0, 1.0, phi));
				vertices.push_back(Vec3(1.0, phi, 0.0));
				vertices.push_back(Vec3(phi, 0.0, 1.0));

				vertices.push_back(Vec3(0.0, -1.0, phi));
				vertices.push_back(Vec3(-1.0, phi, 0.0));
				vertices.push_back(Vec3(phi, 0.0, -1.0));

				vertices.push_back(Vec3(0.0, 1.0, -phi));
				vertices.push_back(Vec3(1.0, -phi, 0.0));
				vertices.push_back(Vec3(-phi, 0.0, 1.0));

				vertices.push_back(Vec3(0.0, 1.0, -phi));
				vertices.push_back(Vec3(1.0, -phi, 0.0));
				vertices.push_back(Vec3(-phi, 0.0, 1.0));
			}
		}

		inline void Dodecahedron(std::vector<Vec3>& vertices, bool one)
		{
			Real phi = (1.0 + sqrt(5.0)) / 2.0;
			Real invphi = 1.0 / phi;

			Cube(vertices);
			if (one)
			{
				vertices.push_back(Vec3(0.0, invphi, phi));
				vertices.push_back(Vec3(invphi, phi, 0.0));
				vertices.push_back(Vec3(phi, 0.0, invphi));

				vertices.push_back(Vec3(0.0, -invphi, phi));
				vertices.push_back(Vec3(-invphi, phi, 0.0));
				vertices.push_back(Vec3(phi, 0.0, -invphi));

				vertices.push_back(Vec3(0.0, -invphi, -phi));
				vertices.push_back(Vec3(-invphi, -phi, 0.0));
				vertices.push_back(Vec3(-phi, 0.0, -invphi));

				vertices.push_back(Vec3(0.0, invphi, -phi));
				vertices.push_back(Vec3(invphi, -phi, 0.0));
				vertices.push_back(Vec3(-phi, 0.0, invphi));
			}
			else
			{
				vertices.push_back(Vec3(0.0, phi, invphi));
				vertices.push_back(Vec3(phi, invphi, 0.0));
				vertices.push_back(Vec3(invphi, 0.0, phi));

				vertices.push_back(Vec3(0.0, phi, -invphi));
				vertices.push_back(Vec3(phi, -invphi, 0.0));
				vertices.push_back(Vec3(-invphi, 0.0, phi));

				vertices.push_back(Vec3(0.0, -phi, -invphi));
				vertices.push_back(Vec3(-phi, -invphi, 0.0));
				vertices.push_back(Vec3(-invphi, 0.0, -phi));

				vertices.push_back(Vec3(0.0, -phi, invphi));
				vertices.push_back(Vec3(-phi, invphi, 0.0));
				vertices.push_back(Vec3(invphi, 0.0, -phi));
			}
		}
	}
}

#endif