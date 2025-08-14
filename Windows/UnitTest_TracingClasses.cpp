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
		// TODO: Test both constructors of RayPencil
		// TODO: Test RayPencil::moveOrigin
		// TODO: Test RayPencil::traceAll
		// TODO: Test RayPencil::clusterDirections

		// TODO: Test both constructors of RayBundle
		// TODO: Implement and test a third contructor of RayBundle, with single origin and hemisphere distribution
		// TODO: Test RayBundle::traceAll
		// TODO: Implement and test RayBundle::advanceAndReflect
		// TODO: Implement and test RayBundle::clusterDirections
	};
}