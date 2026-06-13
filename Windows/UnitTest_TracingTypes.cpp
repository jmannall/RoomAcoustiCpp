#include "CppUnitTest.h"
#include "UtilityFunctions.h"

#include "Spatialiser/Room.h"
#include "Spatialiser/TracingUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Spatialiser;

	TEST_CLASS(TracingTypes)
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

		// Note: these are very approximate distances. The important parts are the sign and validity.
		Real EXPECTED_DIST[2][6][14] = {
		{ /* origin index 0 */
			{1.,  2.,  -1.,   -2.,   3.,  qNaN, REAL_CONST(5e-4), qNaN, qNaN, -11., -12., qNaN, qNaN, qNaN}, /* dir 0 */
			{1.,    qNaN,     qNaN, qNaN, qNaN, qNaN, REAL_CONST(5e-4), qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN}, /* dir 1 */
			{1.,    qNaN, -1.,   -2.,   qNaN, qNaN, REAL_CONST(5e-4), qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN}, /* dir 2 */
			{-1.,  -2.,  1.,    2.,    -3.,  qNaN, REAL_CONST(-5e-4), qNaN, qNaN, 11., 12., qNaN, qNaN, qNaN}, /* dir 3 */
			{qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN}, /* dir 4 */
			{qNaN, qNaN, qNaN, qNaN, qNaN, 2.,   REAL_CONST(-5e-4), qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN}, /* dir 5 */
		},
		{ /* origin index 1 */
			{11., 12., qNaN, qNaN, 13., qNaN, 10.,   1.,    2.,    -1.,  -2.,  3.,    qNaN, REAL_CONST(5e-4)}, /* dir 0 */
			{qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, 1.,    qNaN, qNaN, qNaN, qNaN, qNaN, REAL_CONST(5e-4)}, /* dir 1 */
			{qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, 1.,    qNaN, -1.,   -2.,   qNaN, qNaN, REAL_CONST(5e-4)}, /* dir 2 */
			{-11., -12., qNaN, qNaN, -13., qNaN, -10.,  -1.,   -2.,   1.,  2.,  -3.,   qNaN, REAL_CONST(-5e-4)}, /* dir 3 */
			{qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN}, /* dir 4 */
			{qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, 2.,    REAL_CONST(-5e-4)}, /* dir 5 */
		},
		};
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

	public:
		// Test intersection_test for a single ray.
		TEST_METHOD(SingleRayIntersection)
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
			testOrigins[1] = Vec3({ 0.1, 0.1, 10.0  + 1e-6 });

			Real expected_distance, result_distance, result_cosine;
			for (int oi = 0; oi < 2; ++oi) {
				for (int di = 0; di < 6; ++di) {
					for (int ti = 0; ti < 14; ++ti) {
						expected_distance = EXPECTED_DIST[oi][di][ti];

						result_distance = std::numeric_limits<Real>::quiet_NaN();
						result_cosine = std::numeric_limits<Real>::quiet_NaN();
						const bool success = intersection_test(
							testRoom.GetTriangleMeshSoA(), ti,
							testOrigins[oi], testDirections[di],
							result_distance, result_cosine);

						std::string error = "\nCombination: ";
						error += "\n\torigin " + ToStr(oi) + ", ";
						error += "\n\tdirection " + ToStr(di) + ", ";
						error += "\n\ttriangle " + ToStr(ti) + " | ";
						error += "\nexpected distance " + ToStr(expected_distance) + ", ";
						error += "\nresult distance " + ToStr(result_distance) + ", ";
						error += "\nresult cosine " + ToStr(result_cosine);
						std::wstring werror = std::wstring(error.begin(), error.end());
						const wchar_t* werrorchar = werror.c_str();

						// If we succeed, then the values should not be NaN
						Assert::IsTrue(!success || (!std::isnan(result_cosine) && !std::isnan(result_distance)), werrorchar);
						Assert::IsTrue(success || (std::isnan(result_cosine) && std::isnan(result_distance)), werrorchar);

						// Either both or neither distance should be NaN.
						Assert::AreEqual(std::isnan(expected_distance), std::isnan(result_distance), werrorchar);

						// Either both or neither distance should be finite.
						Assert::AreEqual(std::isnan(expected_distance), !std::isfinite(result_distance), werrorchar);

						// The distances have the same sign (if neither is NaN).
						if (!std::isnan(expected_distance))
							Assert::AreEqual(std::signbit(expected_distance), std::signbit(result_distance), werrorchar);
					}
				}
			}
		}

		// Test trace_ray for a single ray.
		TEST_METHOD(SingleRayTracing)
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

			Real expected_distance_front, result_distance_front, result_cosine_front;
			Real expected_distance_back, result_distance_back, result_cosine_back;
			int expected_idx_front, result_idx_front, expected_idx_back, result_idx_back;
			for (int oi = 0; oi < 2; ++oi) {
				for (int di = 0; di < 6; ++di) {
					expected_distance_front = EXPECTED_DIST_PAIR[oi][di][0];
					expected_distance_back = EXPECTED_DIST_PAIR[oi][di][1];
					expected_idx_front = EXPECTED_IDX_PAIR[oi][di][0];
					expected_idx_back = EXPECTED_IDX_PAIR[oi][di][1];

					trace_ray(
						testRoom.GetTriangleMeshSoA(),
						testOrigins[oi], testDirections[di],
						result_idx_front, result_distance_front, result_cosine_front,
						result_idx_back, result_distance_back, result_cosine_back);
					// TODO: Repeat the test with ignoredTriangleIndex

					std::string error = "\nCombination: ";
					error += "\n\torigin " + ToStr(oi) + ", ";
					error += "\n\tdirection " + ToStr(di) + " | ";
					error += "\nexp. front dist. " + ToStr(expected_distance_front) + ", ";
					error += "\nres. front dist. " + ToStr(result_distance_front) + ", ";
					error += "\nexp. front idx " + ToStr(expected_idx_front) + ", ";
					error += "\nres. front idx " + ToStr(result_idx_front) + ", ";
					error += "\nexp. back dist. " + ToStr(expected_distance_back) + ", ";
					error += "\nres. back dist. " + ToStr(result_distance_back) + ", ";
					error += "\nexp. back idx " + ToStr(expected_idx_back) + ", ";
					error += "\nres. back idx " + ToStr(result_idx_back) + ", ";
					error += "\nres. front cosine " + ToStr(result_cosine_front) + ", ";
					error += "\nres. back cosine " + ToStr(result_cosine_back);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();

					// Either both or neither distance should be NaN.
					Assert::AreEqual(std::isnan(expected_distance_front), std::isnan(result_distance_front), werrorchar);
					Assert::AreEqual(std::isnan(expected_distance_back), std::isnan(result_distance_back), werrorchar);

					// Either both or neither distance should be finite.
					Assert::AreEqual(std::isnan(expected_distance_front), !std::isfinite(result_distance_front), werrorchar);
					Assert::AreEqual(std::isnan(expected_distance_back), !std::isfinite(result_distance_back), werrorchar);

					// The distances have the same sign (if neither is NaN).
					if (!std::isnan(expected_distance_front))
						Assert::AreEqual(std::signbit(expected_distance_front), std::signbit(result_distance_front), werrorchar);
					if (!std::isnan(expected_distance_back))
						Assert::AreEqual(std::signbit(expected_distance_back), std::signbit(result_distance_back), werrorchar);

					// The triangle indices should match expectations.
					Assert::AreEqual(expected_idx_front, result_idx_front, werrorchar);
					Assert::AreEqual(expected_idx_back, result_idx_back, werrorchar);
				}
			}
		}

		// Test intersection_test using a RayPencilSoA.
		TEST_METHOD(PencilSoAIntersection)
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

			RayPencilSoA testRays;
			testRays.resize(6);
			for (int i = 0; i < 6; ++i) {
				testRays.D[i] = testDirections[i];
			}

			Real expected_distance, result_distance, result_cosine;
			for (int oi = 0; oi < 2; ++oi) {
				testRays.O= testOrigins[oi];

				testRays.normalize_directions();

				for (int di = 0; di < 6; ++di) {
					for (int ti = 0; ti < 14; ++ti) {
						expected_distance = EXPECTED_DIST[oi][di][ti];

						result_distance = std::numeric_limits<Real>::quiet_NaN();
						result_cosine = std::numeric_limits<Real>::quiet_NaN();
						const bool success = intersection_test(
							testRoom.GetTriangleMeshSoA(), ti,
							testRays, di,
							result_distance, result_cosine);

						std::string error = "\nCombination: ";
						error += "\n\torigin " + ToStr(oi) + ", ";
						error += "\n\tdirection " + ToStr(di) + ", ";
						error += "\n\ttriangle " + ToStr(ti) + " | ";
						error += "\nexpected distance " + ToStr(expected_distance) + ", ";
						error += "\nresult distance " + ToStr(result_distance) + ", ";
						error += "\nresult cosine " + ToStr(result_cosine);
						std::wstring werror = std::wstring(error.begin(), error.end());
						const wchar_t* werrorchar = werror.c_str();

						// If we succeed, then the values should not be NaN
						Assert::IsTrue(!success || (!std::isnan(result_cosine) && !std::isnan(result_distance)), werrorchar);
						Assert::IsTrue(success || (std::isnan(result_cosine) && std::isnan(result_distance)), werrorchar);

						// Either both or neither distance should be NaN.
						Assert::AreEqual(std::isnan(expected_distance), std::isnan(result_distance), werrorchar);

						// Either both or neither distance should be finite.
						Assert::AreEqual(std::isnan(expected_distance), !std::isfinite(result_distance), werrorchar);

						// The distances have the same sign (if neither is NaN).
						if (!std::isnan(expected_distance))
							Assert::AreEqual(std::signbit(expected_distance), std::signbit(result_distance), werrorchar);
					}
				}
			}
		}
		
		// Test trace_ray using a RayPencilSoA.
		TEST_METHOD(PencilSoATracing)
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

			RayPencilSoA testRays;
			testRays.resize(6);
			for (int i = 0; i < 6; ++i) {
				testRays.D[i] = testDirections[i];
			}

			Real expected_distance_front, result_distance_front, result_cosine_front;
			Real expected_distance_back, result_distance_back, result_cosine_back;
			int expected_idx_front, result_idx_front, expected_idx_back, result_idx_back;
			for (int oi = 0; oi < 2; ++oi) {
				testRays.O = testOrigins[oi];

				testRays.normalize_directions();

				for (int di = 0; di < 6; ++di) {
					expected_distance_front = EXPECTED_DIST_PAIR[oi][di][0];
					expected_distance_back = EXPECTED_DIST_PAIR[oi][di][1];
					expected_idx_front = EXPECTED_IDX_PAIR[oi][di][0];
					expected_idx_back = EXPECTED_IDX_PAIR[oi][di][1];

					trace_ray(
						testRoom.GetTriangleMeshSoA(),
						testRays, di,
						result_idx_front, result_distance_front, result_cosine_front,
						result_idx_back, result_distance_back, result_cosine_back);
					// TODO: Repeat the test with ignoredTriangleIndex

					std::string error = "\nCombination: ";
					error += "\n\torigin " + ToStr(oi) + ", ";
					error += "\n\tdirection " + ToStr(di) + " | ";
					error += "\nexp. front dist. " + ToStr(expected_distance_front) + ", ";
					error += "\nres. front dist. " + ToStr(result_distance_front) + ", ";
					error += "\nexp. front idx " + ToStr(expected_idx_front) + ", ";
					error += "\nres. front idx " + ToStr(result_idx_front) + ", ";
					error += "\nexp. back dist. " + ToStr(expected_distance_back) + ", ";
					error += "\nres. back dist. " + ToStr(result_distance_back) + ", ";
					error += "\nexp. back idx " + ToStr(expected_idx_back) + ", ";
					error += "\nres. back idx " + ToStr(result_idx_back) + ", ";
					error += "\nres. front cosine " + ToStr(result_cosine_front) + ", ";
					error += "\nres. back cosine " + ToStr(result_cosine_back);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();

					// Either both or neither distance should be NaN.
					Assert::AreEqual(std::isnan(expected_distance_front), std::isnan(result_distance_front), werrorchar);
					Assert::AreEqual(std::isnan(expected_distance_back), std::isnan(result_distance_back), werrorchar);

					// Either both or neither distance should be finite.
					Assert::AreEqual(std::isnan(expected_distance_front), !std::isfinite(result_distance_front), werrorchar);
					Assert::AreEqual(std::isnan(expected_distance_back), !std::isfinite(result_distance_back), werrorchar);

					// The distances have the same sign (if neither is NaN).
					if (!std::isnan(expected_distance_front))
						Assert::AreEqual(std::signbit(expected_distance_front), std::signbit(result_distance_front), werrorchar);
					if (!std::isnan(expected_distance_back))
						Assert::AreEqual(std::signbit(expected_distance_back), std::signbit(result_distance_back), werrorchar);

					// The triangle indices should match expectations.
					Assert::AreEqual(expected_idx_front, result_idx_front, werrorchar);
					Assert::AreEqual(expected_idx_back, result_idx_back, werrorchar);
				}
			}
		}
		
		// TODO: Test RayPencilSoA::fill_uniform_sphere (by clustering?)

		// Test intersection_test using a RayBundleSoA.
		TEST_METHOD(BundleSoAIntersection)
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

			RayBundleSoA testRays;
			testRays.resize(12);
			for (int oi = 0; oi < 2; ++oi) {
				for (int di = 0; di < 6; ++di) {
					int ri = di + (oi * 6);
					testRays.O[ri] = testOrigins[oi];
					testRays.D[ri] = testDirections[di];
				}
			}

			testRays.normalize_directions();

			Real expected_distance, result_distance, result_cosine;
			for (int oi = 0; oi < 2; ++oi) {
				for (int di = 0; di < 6; ++di) {
					for (int ti = 0; ti < 14; ++ti) {
						expected_distance = EXPECTED_DIST[oi][di][ti];

						result_distance = std::numeric_limits<Real>::quiet_NaN();
						result_cosine = std::numeric_limits<Real>::quiet_NaN();
						const bool success = intersection_test(
							testRoom.GetTriangleMeshSoA(), ti,
							testRays, di + (oi * 6),
							result_distance, result_cosine);

						std::string error = "\nCombination: ";
						error += "\n\torigin " + ToStr(oi) + ", ";
						error += "\n\tdirection " + ToStr(di) + ", ";
						error += "\n\ttriangle " + ToStr(ti) + " | ";
						error += "\nexpected distance " + ToStr(expected_distance) + ", ";
						error += "\nresult distance " + ToStr(result_distance) + ", ";
						error += "\nresult cosine " + ToStr(result_cosine);
						std::wstring werror = std::wstring(error.begin(), error.end());
						const wchar_t* werrorchar = werror.c_str();

						// If we succeed, then the values should not be NaN
						Assert::IsTrue(!success || (!std::isnan(result_cosine) && !std::isnan(result_distance)), werrorchar);
						Assert::IsTrue(success || (std::isnan(result_cosine) && std::isnan(result_distance)), werrorchar);

						// Either both or neither distance should be NaN.
						Assert::AreEqual(std::isnan(expected_distance), std::isnan(result_distance), werrorchar);

						// Either both or neither distance should be finite.
						Assert::AreEqual(std::isnan(expected_distance), !std::isfinite(result_distance), werrorchar);

						// The distances have the same sign (if neither is NaN).
						if (!std::isnan(expected_distance))
							Assert::AreEqual(std::signbit(expected_distance), std::signbit(result_distance), werrorchar);
					}
				}
			}
		}

		// Test trace_ray using a RayBundleSoA.
		TEST_METHOD(BundleSoATracing)
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

			RayBundleSoA testRays;
			testRays.resize(12);
			for (int oi = 0; oi < 2; ++oi) {
				for (int di = 0; di < 6; ++di) {
					int ri = di + (oi * 6);
					testRays.O[ri] = testOrigins[oi];
					testRays.D[ri] = testDirections[di];
				}
			}

			testRays.normalize_directions();

			Real expected_distance_front, result_distance_front, result_cosine_front;
			Real expected_distance_back, result_distance_back, result_cosine_back;
			int expected_idx_front, result_idx_front, expected_idx_back, result_idx_back;
			for (int oi = 0; oi < 2; ++oi) {
				for (int di = 0; di < 6; ++di) {
					expected_distance_front = EXPECTED_DIST_PAIR[oi][di][0];
					expected_distance_back = EXPECTED_DIST_PAIR[oi][di][1];
					expected_idx_front = EXPECTED_IDX_PAIR[oi][di][0];
					expected_idx_back = EXPECTED_IDX_PAIR[oi][di][1];

					trace_ray(
						testRoom.GetTriangleMeshSoA(),
						testRays, di + (oi * 6),
						result_idx_front, result_distance_front, result_cosine_front,
						result_idx_back, result_distance_back, result_cosine_back);
					// TODO: Repeat the test with ignoredTriangleIndex

					std::string error = "\nCombination: ";
					error += "\n\torigin " + ToStr(oi) + ", ";
					error += "\n\tdirection " + ToStr(di) + " | ";
					error += "\nexp. front dist. " + ToStr(expected_distance_front) + ", ";
					error += "\nres. front dist. " + ToStr(result_distance_front) + ", ";
					error += "\nexp. front idx " + ToStr(expected_idx_front) + ", ";
					error += "\nres. front idx " + ToStr(result_idx_front) + ", ";
					error += "\nexp. back dist. " + ToStr(expected_distance_back) + ", ";
					error += "\nres. back dist. " + ToStr(result_distance_back) + ", ";
					error += "\nexp. back idx " + ToStr(expected_idx_back) + ", ";
					error += "\nres. back idx " + ToStr(result_idx_back) + ", ";
					error += "\nres. front cosine " + ToStr(result_cosine_front) + ", ";
					error += "\nres. back cosine " + ToStr(result_cosine_back);
					std::wstring werror = std::wstring(error.begin(), error.end());
					const wchar_t* werrorchar = werror.c_str();

					// Either both or neither distance should be NaN.
					Assert::AreEqual(std::isnan(expected_distance_front), std::isnan(result_distance_front), werrorchar);
					Assert::AreEqual(std::isnan(expected_distance_back), std::isnan(result_distance_back), werrorchar);

					// Either both or neither distance should be finite.
					Assert::AreEqual(std::isnan(expected_distance_front), !std::isfinite(result_distance_front), werrorchar);
					Assert::AreEqual(std::isnan(expected_distance_back), !std::isfinite(result_distance_back), werrorchar);

					// The distances have the same sign (if neither is NaN).
					if (!std::isnan(expected_distance_front))
						Assert::AreEqual(std::signbit(expected_distance_front), std::signbit(result_distance_front), werrorchar);
					if (!std::isnan(expected_distance_back))
						Assert::AreEqual(std::signbit(expected_distance_back), std::signbit(result_distance_back), werrorchar);

					// The triangle indices should match expectations.
					Assert::AreEqual(expected_idx_front, result_idx_front, werrorchar);
					Assert::AreEqual(expected_idx_back, result_idx_back, werrorchar);
				}
			}
		}

		// TODO: Test RayBundleSoA::fill_uniform_sphere (by clustering?)

		// TODO: Make sure surface normal vectors are normalized at all times.
		// TODO: Make sure any other "assumed unit" vectors are normalized at all times.
	};
}