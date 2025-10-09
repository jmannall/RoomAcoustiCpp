#include "CppUnitTest.h"
#include "UtilityFunctions.h"

#include "Common/Definitions.h"
#include "Common/Vec3.h"

#include "Spatialiser/Room.h"
#include "Spatialiser/TracingUtils.h"

#include <numeric>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Spatialiser;

	TEST_CLASS(TracingClasses)
	{
		// Build a room containing an assortment of triangles
		void buildTestMesh(Room& testRoom) {
			std::vector<Vec3> allVertices;
			allVertices.resize(42);
			allVertices[0] = Vec3({ 0.0, 0.0, -1.0 });
			allVertices[1] = Vec3({ 1.0, 0.0, -1.0 });
			allVertices[2] = Vec3({ 0.0, 1.0, -1.0 });
			allVertices[3] = Vec3({ 0.0, 0.0, -2.0 });
			allVertices[4] = Vec3({ 2.0, 0.0, -2.0 });
			allVertices[5] = Vec3({ 0.0, 2.0, -2.0 });
			allVertices[6] = Vec3({ 0.0, 0.0, 1.0 });
			allVertices[7] = Vec3({ 0.0, 1.0, 1.0 });
			allVertices[8] = Vec3({ 1.0, 0.0, 1.0 });
			allVertices[9] = Vec3({ 0.0, 0.0, 2.0 });
			allVertices[10] = Vec3({ 0.0, 2.0, 2.0 });
			allVertices[11] = Vec3({ 2.0, 0.0, 2.0 });
			allVertices[12] = Vec3({ 0.0, 0.0, -3.0 });
			allVertices[13] = Vec3({ 1.0, 0.0, -3.0 });
			allVertices[14] = Vec3({ 0.0, 1.0, -3.0 });
			allVertices[15] = Vec3({ 1.2, 1.5, -0.7 });
			allVertices[16] = Vec3({ 2.0, 1.5, -0.7 });
			allVertices[17] = Vec3({ 1.2, 1.5, 1.3 });
			allVertices[18] = Vec3({ 0.0, 0.0, 0.0 });
			allVertices[19] = Vec3({ 1.0, 0.0, 0.0 });
			allVertices[20] = Vec3({ 0.0, 1.0, 0.0 });
			allVertices[21] = Vec3({ 0.0, 0.0, 9.0 });
			allVertices[22] = Vec3({ 1.0, 0.0, 9.0 });
			allVertices[23] = Vec3({ 0.0, 1.0, 9.0 });
			allVertices[24] = Vec3({ 0.0, 0.0, 8.0 });
			allVertices[25] = Vec3({ 2.0, 0.0, 8.0 });
			allVertices[26] = Vec3({ 0.0, 2.0, 8.0 });
			allVertices[27] = Vec3({ 0.0, 0.0, 11.0 });
			allVertices[28] = Vec3({ 0.0, 1.0, 11.0 });
			allVertices[29] = Vec3({ 1.0, 0.0, 11.0 });
			allVertices[30] = Vec3({ 0.0, 0.0, 12.0 });
			allVertices[31] = Vec3({ 0.0, 2.0, 12.0 });
			allVertices[32] = Vec3({ 2.0, 0.0, 12.0 });
			allVertices[33] = Vec3({ 0.0, 0.0, 7.0 });
			allVertices[34] = Vec3({ 1.0, 0.0, 7.0 });
			allVertices[35] = Vec3({ 0.0, 1.0, 7.0 });
			allVertices[36] = Vec3({ 1.2, 1.5, 9.3 });
			allVertices[37] = Vec3({ 2.0, 1.5, 9.3 });
			allVertices[38] = Vec3({ 1.2, 1.5, 11.3 });
			allVertices[39] = Vec3({ 0.0, 0.0, 10.0 });
			allVertices[40] = Vec3({ 1.0, 0.0, 10.0 });
			allVertices[41] = Vec3({ 0.0, 1.0, 10.0 });

			Vertices testVertices;
			Coefficients<> testAbsorption = Coefficients<>::Constant(1, 0.5);

			int node = 0;
			for (int i = 0; i < 40; i += 3) {
				testVertices = Vertices({ allVertices[i], allVertices[i + 1], allVertices[i + 2] });
				testRoom.InitMaterial(testAbsorption);
				Wall testWall(testVertices, node++);
				testRoom.AddWall(testWall);
			}
		}

		// Note: these are very approximate distances. The important parts are the triangle index and validity.
		Real EXPECTED_DIST_PAIR[2][6][2] = {
		{ /* origin index 0 */
			{1., 1.}, /* dir 0 */
			{1., qNaN}, /* dir 1 */
			{1., 1.}, /* dir 2 */
			{1., 1.}, /* dir 3 */
			{qNaN, qNaN}, /* dir 4 */
			{2., qNaN}, /* dir 5 */
		},
		{ /* origin index 1 */
			{1., 1.}, /* dir 0 */
			{1., qNaN}, /* dir 1 */
			{1., 1.}, /* dir 2 */
			{1., 1.}, /* dir 3 */
			{qNaN, qNaN}, /* dir 4 */
			{2., qNaN}, /* dir 5 */
		},
		};
		int EXPECTED_IDX_PAIR[2][6][2] = {
		{ /* origin index 0 */
			{0, 2}, /* dir 0 */
			{0, -1}, /* dir 1 */
			{0, 2}, /* dir 2 */
			{2, 0}, /* dir 3 */
			{-1, -1}, /* dir 4 */
			{5, -1}, /* dir 5 */
		},
		{ /* origin index 1 */
			{7, 9}, /* dir 0 */
			{7, -1}, /* dir 1 */
			{7, 9}, /* dir 2 */
			{9, 7}, /* dir 3 */
			{-1, -1}, /* dir 4 */
			{12, -1}, /* dir 5 */
		},
		};

		// TODO: See if this code already exists (e.g., for reverb sources). Consider replacing either one or the other.
		void PlatonicVertices(int n, std::vector<Vec3>& verts) {
			verts.resize(n);

			switch (n) {
			case 1:
				verts[0] = Vec3({ 0.0, 0.0, 1.0 });
				return;

			case 2:
				verts[0] = Vec3({ 0.0, 0.0, 1.0 });
				verts[1] = Vec3({ 0.0, 0.0, -1.0 });
				return;

			case 3: {
				// Equilateral triangle on the equator (great circle)
				for (int k = 0; k < 3; ++k) {
					Real theta = PI_2 * k / 3.0;
					verts[k] = Vec3(std::cos(theta), std::sin(theta), (Real)0.0);
				}
				return;
			}

			case 4: {
				// Regular tetrahedron: permutations of (+-1, +-1, +-1) with an odd number of negatives
				// Normalize by sqrt(3) to lie on the unit sphere
				const Real s = INV_SQRT_3;
				verts[0] = Vec3({ s, s, s });
				verts[1] = Vec3({ s, -s, -s });
				verts[2] = Vec3({ -s, s, -s });
				verts[3] = Vec3({ -s, -s, s });
				return;
			}

			case 6: {
				// Regular octahedron: coordinate axes
				verts[0] = Vec3({ 1.0, 0.0, 0.0 });
				verts[1] = Vec3({ -1.0, 0.0, 0.0 });
				verts[2] = Vec3({ 0.0, 1.0, 0.0 });
				verts[3] = Vec3({ 0.0, -1.0, 0.0 });
				verts[4] = Vec3({ 0.0, 0.0, 1.0 });
				verts[5] = Vec3({ 0.0, 0.0, -1.0 });
				return;
			}

			case 8: {
				// Cube: all (+-1, +-1, +-1), normalized by sqrt(3)
				const Real s = INV_SQRT_3;
				int i = 0;
				for (int sx : {-1, 1})
				{
					for (int sy : {-1, 1})
					{
						for (int sz : {-1, 1})
						{
							verts[i] = Vec3({ sx * s, sy * s, sz * s });
							++i;
						}
					}
				}
				return;
			}

			case 12: {
				// Icosahedron: (0, +-1, +-phi), (+-1, +-phi, 0), (+-phi, 0, +-1)
				// Circumradius of raw coords is sqrt(1 + phi^2); normalize to unit sphere
				const Real r = std::sqrt(1.0 + PHI * PHI);
				const Real s1 = 1.0 / r;
				const Real sP = PHI / r;

				int i = 0;
				// (0, +-1, +-phi)
				for (int sy : {-1, 1})
				{
					for (int sz : {-1, 1})
					{
						verts[i] = Vec3((Real)0.0, sy * s1, sz * sP);
						++i;
					}
				}
				// (+-1, +-phi, 0)
				for (int sx : {-1, 1})
				{
					for (int sy : {-1, 1})
					{
						verts[i] = Vec3(sx * s1, sy * sP, (Real)0.0);
						++i;
					}
				}
				// (+-phi, 0, +-1)
				for (int sx : {-1, 1})
				{
					for (int sz : {-1, 1})
					{
						verts[i] = Vec3(sx * sP, (Real)0.0, sz * s1);
						++i;
					}
				}
				return;
			}

			case 20: {
				// Dodecahedron: vertices are permutations of
				// (+-1, +-1, +-1),
				// (0, +-1/phi, +-phi),
				// (+-1/phi, +-phi, 0),
				// (+-phi, 0, +-1/phi).
				// All these raw points have radius sqrt(3); normalize by sqrt(3).
				const Real s = INV_SQRT_3;

				int i = 0;

				// (+-1, +-1, +-1)
				for (int sx : {-1, 1})
				{
					for (int sy : {-1, 1})
					{
						for (int sz : {-1, 1})
						{
							verts[i] = Vec3({ sx * s, sy * s, sz * s });
							++i;
						}
					}
				}

				// (0, +-1/phi, +-phi)
				for (int sy : {-1, 1})
				{
					for (int sz : {-1, 1})
					{
						verts[i] = Vec3((Real)0.0, INV_PHI * sy * s, PHI * sz * s);
						++i;
					}
				}

				// (+-1/phi, +-phi, 0)
				for (int sx : {-1, 1})
				{
					for (int sy : {-1, 1})
					{
						verts[i] = Vec3(INV_PHI * sx * s, PHI * sy * s, (Real)0.0);
						++i;
					}
				}

				// (+-phi, 0, +-1/phi)
				for (int sx : {-1, 1})
				{
					for (int sz : {-1, 1})
					{
						verts[i] = Vec3(PHI * sx * s, (Real)0.0, INV_PHI * sz * s);
						++i;
					}
				}

				return;
			}

			default:
				verts.resize(0);
				return;
			}
		}

	public:
		// Test RayPencil::moveOrigin, RayPencil::traceAll, RayPencil direction normalization upon construction, and RayPencil `get` methods.
		TEST_METHOD(PencilClassTracing)
		{
			Room testRoom(1);
			buildTestMesh(testRoom);
			testRoom.CreateTriangleMeshSoA();
			Assert::AreEqual(14, testRoom.GetTriangleMeshSoA().size(), L"\nThe test room does not contain 14 triangles.");

			std::vector<Vec3> testDirections;
			testDirections.resize(6);
			testDirections[0] = Vec3({ 0.0, 0.0, -1.0 });
			testDirections[1] = Vec3({ 0.4, -0.1, -1.0 });
			testDirections[2] = Vec3({ -0.1, -0.1, -1.0 });
			testDirections[3] = Vec3({ 0.0, 0.0, 1.0 });
			testDirections[4] = Vec3({ 1.0, 0.0, 0.0 });
			testDirections[5] = Vec3({ 10.0, 10.0, 1.0 });

			std::vector<Vec3> testOrigins;
			testOrigins.resize(2);
			testOrigins[0] = Vec3({ 0.1, 0.1, 1e-6 });
			testOrigins[1] = Vec3({ 0.1, 0.1, 10.0 + 1e-6 });

			RayPencil testPencil(testDirections);

			// Check that directions are automatically normalized on construction.
			std::vector<Vec3> actualDirections(testDirections.size());
			testPencil.getDirections(actualDirections);
			for (int di = 0; di < actualDirections.size(); ++di) {
				Real magnitude = norm3(actualDirections[di].x(), actualDirections[di].y(), actualDirections[di].z());

				std::string error = "\nDirection vector " + ToStr(di) + " is not normalized.";
				std::wstring werror = std::wstring(error.begin(), error.end());
				const wchar_t* werrorchar = werror.c_str();

				Assert::AreEqual((Real)1.0, magnitude, (Real)1e-6, werrorchar);
			}

			Vec<> rayDistances(SizeToInt(testDirections.size()));
			Vec<> rayCosines(SizeToInt(testDirections.size()));
			Vec<int> frontIndices(SizeToInt(testDirections.size()));
			Vec<int> backIndices(SizeToInt(testDirections.size()));
			Real expected_distance, result_distance, result_cosine;
			int expected_idx_front, result_idx_front, expected_idx_back, result_idx_back;
			for (int oi = 0; oi < 2; ++oi) {
				testPencil.moveOrigin(testOrigins[oi]);
				testPencil.traceAll(testRoom.GetTriangleMeshSoA());

				testPencil.getDistances(rayDistances);
				testPencil.getCosines(rayCosines);
				testPencil.getIndices(frontIndices, backIndices);

				for (int di = 0; di < 6; ++di) {
					expected_distance = EXPECTED_DIST_PAIR[oi][di][0];
					expected_idx_front = EXPECTED_IDX_PAIR[oi][di][0];
					expected_idx_back = EXPECTED_IDX_PAIR[oi][di][1];

					result_idx_front = frontIndices(di);
					result_distance = rayDistances(di);
					result_cosine = rayCosines(di);
					result_idx_back = backIndices(di);

					std::string error = "\nCombination: ";
					error += "\n\torigin " + ToStr(oi) + ", ";
					error += "\n\tdirection " + ToStr(di) + " | ";
					error += "\nexp. dist. " + ToStr(expected_distance) + ", ";
					error += "\nres. dist. " + ToStr(result_distance) + ", ";
					error += "\nexp. front idx " + ToStr(expected_idx_front) + ", ";
					error += "\nres. front idx " + ToStr(result_idx_front) + ", ";
					error += "\nexp. back idx " + ToStr(expected_idx_back) + ", ";
					error += "\nres. back idx " + ToStr(result_idx_back) + ", ";
					error += "\nres. cosine " + ToStr(result_cosine);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();

					// Either both or neither distance should be NaN.
					Assert::AreEqual(std::isnan(expected_distance), std::isnan(result_distance), werrorchar);

					// Either both or neither distance should be finite.
					Assert::AreEqual(std::isnan(expected_distance), !std::isfinite(result_distance), werrorchar);

					// The distances have the same sign (if neither is NaN).
					if (!std::isnan(expected_distance))
						Assert::AreEqual(std::signbit(expected_distance), std::signbit(result_distance), werrorchar);

					// The triangle indices should match expectations.
					Assert::AreEqual(expected_idx_front, result_idx_front, werrorchar);
					Assert::AreEqual(expected_idx_back, result_idx_back, werrorchar);
				}
			}
		}

		// Test RayPencil::clusterDirections
		TEST_METHOD(PencilClassClustering)
		{
			Vec3 xDir({ 1.0, 0.0, 0.0 });
			Vec3 yDir({ 0.0, 1.0, 0.0 });
			Vec3 zDir({ 0.0, 0.0, 1.0 });

			std::vector<Vec3> testDirections, tempDirections;
			Vec<int> rayClusters = Vec<int>::Constant(1, -1);

			RayPencil testPencil(0);

			// Test with one ray and multiple reference directions (basic assignment).
			testDirections.resize(1);
			testDirections[0] = Vec3({ 2.0, 0.1, 0.1 });

			testPencil = RayPencil(testDirections);

			testDirections.resize(3);
			testDirections[0] = xDir;
			testDirections[1] = yDir;
			testDirections[2] = zDir;

			testPencil.clusterDirections(testDirections, rayClusters);

			Assert::AreEqual(0, rayClusters(0), L"\nExpected: first direction\n(single ray).");

			// Test with multiple rays and zero reference directions (all outputs == -1).
			testDirections.resize(6);
			testDirections[0] = xDir;
			testDirections[1] = yDir;
			testDirections[2] = zDir;
			testDirections[3] = -xDir;
			testDirections[4] = -yDir;
			testDirections[5] = -zDir;

			testPencil = RayPencil(testDirections);
			rayClusters = std::vector<int>(testDirections.size(), -1);

			testDirections.resize(0);

			testPencil.clusterDirections(testDirections, rayClusters);

			for (int di = 0; di < rayClusters.Length(); ++di)
				Assert::AreEqual(-1, rayClusters(di), L"\nExpected: no direction\n(empty dir).");
			
			// Test with multiple rays and one reference direction (basic front-back choice).
			testDirections.resize(6);
			testDirections[0] = xDir;
			testDirections[1] = yDir;
			testDirections[2] = zDir;
			testDirections[3] = -xDir;
			testDirections[4] = -yDir;
			testDirections[5] = -zDir;

			testPencil = RayPencil(testDirections);
			rayClusters = Vec<int>::Constant(SizeToInt(testDirections.size()), -1);

			testDirections.resize(1);
			testDirections[0] = (xDir + yDir + zDir).Normalised();

			testPencil.clusterDirections(testDirections, rayClusters);

			for (int di = 0; di < rayClusters.Length(); ++di)
				Assert::AreEqual(0, rayClusters(di), L"\nExpected: first direction\n(single dir).");
			
			// Test with multiple rays and multiple reference directions (all following tests).

			// Test with ray directions that exactly match reference directions.
			testDirections.resize(10);
			testDirections[0] = xDir;
			testDirections[1] = yDir;
			testDirections[2] = zDir;
			testDirections[3] = (xDir + yDir).Normalised();
			testDirections[4] = (xDir + zDir).Normalised();
			testDirections[5] = (yDir + zDir).Normalised();
			testDirections[6] = (xDir + yDir + zDir).Normalised();
			testDirections[7] = (-0.5 * xDir + yDir + zDir).Normalised();
			testDirections[8] = (xDir - 0.5 * yDir + zDir).Normalised();
			testDirections[9] = (xDir + yDir - 0.5 * zDir).Normalised();

			// Scramble the signs to get a more "random" set of test directions.
			for (int di = 0; di < testDirections.size(); di += 2)
				testDirections[di] *= -1;

			testPencil = RayPencil(testDirections);
			rayClusters = std::vector<int>(testDirections.size(), -1);

			testPencil.clusterDirections(testDirections, rayClusters);

			for (int di = 0; di < rayClusters.Length(); ++di)
				Assert::AreEqual(di, rayClusters(di), L"\nExpected: direction with ray's index (identical).");

			// Wiggle the directions a bit and repeat the test.
			for (int di = 0; di < testDirections.size(); ++di)
			{
				testDirections[di] += Vec3({ 1e-2, 1e-2, 1e-2 });
				testDirections[di].Normalise();
			}

			testPencil.clusterDirections(testDirections, rayClusters);

			for (int di = 0; di < testDirections.size(); ++di)
				Assert::AreEqual(di, rayClusters(di), L"\nExpected: direction with ray's index (wiggled).");

			// Try with more rays than directions.
			// Note: this code should work without modification even if you change the size of testDirections, defined above.
			int chunkSize = SizeToInt(testDirections.size());
			tempDirections.resize(9 * chunkSize);
			for (int di = 0; di < chunkSize; ++di)
				tempDirections[di] = testDirections[di];

			int chunkStart = SizeToInt(testDirections.size());
			for (int sx : {-1, 1})
			{
				for (int sy : {-1, 1})
				{
					for (int sz : {-1, 1})
					{
						Vec3 offset = Vec3({ sx * 1e-2, sy * 1e-2, sz * 1e-2 });
						for (int di = 0; di < chunkSize; ++di)
							tempDirections[di + chunkStart] = (testDirections[di] + offset).Normalised();
						chunkStart += SizeToInt(testDirections.size());
					}
				}
			}

			testPencil = RayPencil(tempDirections);
			rayClusters = std::vector<int>(tempDirections.size(), -1);

			testPencil.clusterDirections(testDirections, rayClusters);

			for (int di = 0; di < rayClusters.Length(); ++di)
				Assert::AreEqual(di % chunkSize, rayClusters(di), L"\nExpected: direction with ray's index mod chunkSize (more rays than dirs).");

			// TODO: Test with non-normalized directions to ensure the cosine similarity calculation is still meaningful (weighted clustering).
		}

		// Test hemisphere constructor of RayPencil, using directions clustering (validated in previous test)
		TEST_METHOD(PencilClassHemisphere)
		{
			std::vector<Vec3> testDirections;
			int effectiveNumRays;

			for (int numRays = 100; numRays < 10001; numRays *= 10)
			{
				for (int numClusters : {1, 2, 3, 4, 6, 8, 12, 20})
				{
					std::vector<int> tally(numClusters);
					PlatonicVertices(numClusters, testDirections);

					for (bool hemisphereOnly : { true, false })
					{
						RayPencil testPencil(numRays, hemisphereOnly);

						effectiveNumRays = testPencil.getNumRays();

						Vec<int> rayClusters = Vec<int>::Constant(effectiveNumRays, -1);
						testPencil.clusterDirections(testDirections, rayClusters);

						for (int ci = 0; ci < numClusters; ++ci)
						{
							tally[ci] = 0;
							for (int ri = 0; ri < effectiveNumRays; ++ri)
								if (rayClusters(ri) == ci)
									tally[ci] += 1;
						}

						// Check that the sum of all values in `tally` is equal to `numRays`.
						int tallyTotal = std::reduce(tally.begin(), tally.end());
						Assert::AreEqual(effectiveNumRays, tallyTotal, L"\nThe total cluster tally does not contain all rays.");

						// Check that the difference between the minimum and maximum of `tally` is relatively small
						int tallyMax = *std::max_element(tally.begin(), tally.end());
						int tallyMin = *std::min_element(tally.begin(), tally.end());

						std::string error = "\nThe cluster sizes vary by more than 10% of all rays.";
						error += "\nMax cluster " + ToStr(tallyMax) + ", ";
						error += "\nMin cluster " + ToStr(tallyMin) + ", ";
						error += "\nNum rays " + ToStr(effectiveNumRays) + ".";
						std::wstring werror = std::wstring(error.begin(), error.end());
						const wchar_t* werrorchar = werror.c_str();

						Assert::IsTrue(std::abs(tallyMax - tallyMin) <= (effectiveNumRays / 10), werrorchar);
					}
				}
			}
		}

		// TODO: Test RayBundle::traceAll
		// TODO: Implement and test RayBundle::advanceAndReflect
		// TODO: Implement and test RayBundle::clusterDirections
		// TODO: Implement and test a third contructor of RayBundle, with single origin and hemisphere distribution
	};
}