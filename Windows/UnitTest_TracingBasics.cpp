
#include "CppUnitTest.h"
#include "UtilityFunctions.h"

#include "Spatialiser/Room.h"
#include "Spatialiser/TracingUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Spatialiser;

	TEST_CLASS(TracingTypeBasics)
	{
		// Build a room containing an assortment of triangles
		void buildTestMesh(Room& tRoom) {
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

			Wall tWall;
			Vertices tVertices;
			Absorption<> tAbsorption(0.5);
			for (int i = 0; i < 40; i += 3) {
				tVertices = Vertices({ allVertices[i], allVertices[i + 1], allVertices[i + 2] });
				tWall = Wall(tVertices, tAbsorption);
				tRoom.AddWall(tWall);
			}
		}

		// Note: these are very approximate distances. The important parts are the sign and validity.
		Real EXPECTED_DIST[2][6][14] = {
		{ /* origin index 0 */
				{1.,  2.,  -1.,   -2.,   3.,  qNaN, 5e-4, qNaN, qNaN, -11., -12., qNaN, qNaN, qNaN}, /* dir 0 */
				{1.,    qNaN,     qNaN, qNaN, qNaN, qNaN, 5e-4, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN}, /* dir 1 */
				{1.,    qNaN, -1.,   -2.,   qNaN, qNaN, 5e-4, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN}, /* dir 2 */
				{-1.,  -2.,  1.,    2.,    -3.,  qNaN, -5e-4, qNaN, qNaN, 11., 12., qNaN, qNaN, qNaN}, /* dir 3 */
				{qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN}, /* dir 4 */
				{qNaN, qNaN, qNaN, qNaN, qNaN, 1.,    -5e-4, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN}, /* dir 5 */
		},
		{ /* origin index 1 */
				{11., 12., qNaN, qNaN, 13., qNaN, 10.,   1.,    2.,    -1.,  -2.,  3.,    qNaN, 5e-4}, /* dir 0 */
				{qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, 1.,    qNaN, qNaN, qNaN, qNaN, qNaN, 5e-4}, /* dir 1 */
				{qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, 1.,    qNaN, -1.,   -2.,   qNaN, qNaN, 5e-4}, /* dir 2 */
				{-11., -12., qNaN, qNaN, -13., qNaN, -10.,  -1.,   -2.,   1.,  2.,  -3.,   qNaN, -5e-4}, /* dir 3 */
				{qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN}, /* dir 4 */
				{qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, qNaN, 1.,    -5e-4}, /* dir 5 */
		},
		};

	public:
		// Test kernels for a single ray.
		TEST_METHOD(SingleRayKernels)
		{
			Room tRoom(1);
			buildTestMesh(tRoom);
			tRoom.CreateTriangleMeshSoA();
			Assert::AreEqual(14, tRoom.GetTriangleMeshSoA().size(), L"The test room does not contain 14 triangles.");

			std::vector<Vec3> tDirections;
			tDirections.resize(6);
			tDirections[0] = Vec3({ 0.0, 0.0, -1.0 });
			tDirections[1] = Vec3({ 0.4, -0.1, -1.0 });
			tDirections[2] = Vec3({ -0.1, -0.1, -1.0 });
			tDirections[3] = Vec3({ 0.0, 0.0, 1.0 });
			tDirections[4] = Vec3({ 1.0, 0.0, 0.0 });
			tDirections[5] = Vec3({ 10.0, 10.0, 1.0 });

			std::vector<Vec3> tOrigins;
			tOrigins.resize(2);
			tOrigins[0] = Vec3({ 0.1, 0.1, 1e-6 });
			tOrigins[1] = Vec3({ 0.1, 0.1, 10.0  + 1e-6 });

			Real expected_distance, result_distance;
			Real expected_cosine, result_cosine;
			for (int oi = 1; oi < 2; ++oi) {
				for (int di = 0; di < 6; ++di) {
					for (int ti = 13; ti < 14; ++ti) {
						expected_distance = EXPECTED_DIST[oi][di][ti];
						//TODO: expected_cosine = EXPECTED_COS[oi][di][ti];

						intersection_test(
							tRoom.GetTriangleMeshSoA(), ti,
							tOrigins[oi], tDirections[di],
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

		// TODO: Test kernels for a RayPencilSoA.

		// TODO: Test kernels for a RayBundleSoA.

		// TODO: Test methods of RayPencil.
		//RayPencil tPencil(tDirections);
		//tPencil.moveOrigin(tOrigins[0]);
		//tPencil.traceAll(tRoom.GetTriangleMeshSoA());
		
		// TODO: Test methods of RayBundle.
	};
}