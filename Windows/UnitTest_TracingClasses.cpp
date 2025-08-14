#include "CppUnitTest.h"
#include "UtilityFunctions.h"

#include "Spatialiser/Room.h"
#include "Spatialiser/TracingUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Spatialiser;

	TEST_CLASS(TracingClasses)
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
		// Test RayPencil::moveOrigin, RayPencil::traceAll, and RayPencil `get` methods.
		TEST_METHOD(PencilClassTracing)
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
			tOrigins[1] = Vec3({ 0.1, 0.1, 10.0 + 1e-6 });

			RayPencil tPencil(tDirections);

			Vec<Real> frontDistances, backDistances, frontCosines, backCosines;
			std::vector<int> frontIndices, backIndices;
			Real expected_distance_front, result_distance_front, result_cosine_front;
			Real expected_distance_back, result_distance_back, result_cosine_back;
			int expected_idx_front, result_idx_front, expected_idx_back, result_idx_back;
			for (int oi = 0; oi < 2; ++oi) {
				tPencil.moveOrigin(tOrigins[oi]);
				tPencil.traceAll(tRoom.GetTriangleMeshSoA());

				tPencil.getDistances(frontDistances, backDistances);
				tPencil.getIndices(frontIndices, backIndices);
				tPencil.getCosines(frontCosines, backCosines);

				for (int di = 0; di < 6; ++di) {
					expected_distance_front = EXPECTED_DIST_PAIR[oi][di][0];
					expected_distance_back = EXPECTED_DIST_PAIR[oi][di][1];
					expected_idx_front = EXPECTED_IDX_PAIR[oi][di][0];
					expected_idx_back = EXPECTED_IDX_PAIR[oi][di][1];

					result_idx_front = frontIndices[di];
					result_distance_front = frontDistances[di];
					result_cosine_front = frontCosines[di];
					result_idx_back = backIndices[di];
					result_distance_back = backDistances[di];
					result_cosine_back = backCosines[di];

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

		// TODO: Make sure direction vectors are normalized in all relevant places.
		
		// TODO: Test RayPencil::clusterDirections
		// TODO: Perfect Forward Match Test: Test with ray directions that exactly match reference directions (dot product = 1.0). Verify that the matching ray gets assigned to the correct reference direction index in frontClusters and -1 in backClusters.
		// TODO: Perfect Backward Match Test : Test with ray directions that are exactly opposite to reference directions(dot product = -1.0). Verify that the matching ray gets - 1 in frontClusters and the correct reference direction index in backClusters.
		// TODO: Mixed Forward / Backward Assignment Test : Test with a mix of rays where some match forward directions and others match backward directions, ensuring the correct clustering assignment logic.
		// TODO: Single Reference Direction Test : Test with only one reference direction to ensure the algorithm works with minimal input.
		// TODO: Single Ray Test : Test with only one ray in the pencil to ensure the algorithm handles minimal ray counts.
		// TODO: Empty Reference Directions Test : Test behavior when the reference directions vector is empty(should handle gracefully or assert).
		// TODO: Near - Zero Cosine Values Test : Test with very small positive and negative cosine similarities(e.g., ±1e - 15) to ensure floating - point precision doesn't cause incorrect clustering.
		// TODO: Large Magnitude Directions Test : Test with non - normalized directions that have large magnitudes to verify the dot product calculation remains stable.
		// TODO: Denormalized Input Directions Test : Test with reference directions that aren't unit vectors to ensure the cosine similarity calculation is still meaningful.
		// TODO: All Rays Forward Clustering Test : Test scenarios where all rays should be assigned to forward clusters(no ray should get backward assignment).
		// TODO: All Rays Backward Clustering Test : Test scenarios where all rays should be assigned to backward clusters(no ray should get forward assignment).
		// TODO: Multiple Rays to Same Reference Test : Test multiple rays that should all cluster to the same reference direction, ensuring consistent assignment.
		// TODO: Uninitialized Ray Data Test : Test behavior when the RayPencilSoA contains uninitialized or NaN values.
		// TODO: Symmetry Test : For any ray direction d, if it clusters to reference r in forward, then direction - d should cluster to r in backward.
		// TODO: Closest Match Verification Test : For each ray, verify that the assigned reference direction actually has the highest absolute cosine similarity among all reference directions.
		// TODO: Mutual Exclusivity Test : Verify that for every ray index i, exactly one of frontClusters[i] == -1 or backClusters[i] == -1 is true (never both - 1 or both valid indices).
		
		// TODO: Test hemisphere constructor of RayPencil, using the already validated clusterDirections

		// TODO: Test RayBundle::traceAll
		// TODO: Implement and test RayBundle::advanceAndReflect
		// TODO: Implement and test RayBundle::clusterDirections
		// TODO: Implement and test a third contructor of RayBundle, with single origin and hemisphere distribution
	};
}