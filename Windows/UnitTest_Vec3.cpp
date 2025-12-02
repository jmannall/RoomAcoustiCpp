
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "Common/Vec3.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Common;

#pragma optimize("", off)

	TEST_CLASS(Vec3_Class)
	{
	public:

		TEST_METHOD(Assign)
		{
			Real x = REAL_CONST(1.0);
			Real y = REAL_CONST(-2.0);
			Real z = REAL_CONST(3.0);
			Vec3 vec;

			vec.x() = x;
			vec.y() = y;
			vec.z() = z;

			Assert::AreEqual(x, vec.x(), L"Error: Add x entry");
			Assert::AreEqual(y, vec.y(), L"Error: Add y entry");
			Assert::AreEqual(z, vec.z(), L"Error: Add z entry");
		}

		TEST_METHOD(Init)
		{
			Real x = REAL_CONST(0.8);
			Real y = REAL_CONST(7.1);
			Real z = REAL_CONST(-0.2);
			Vec3 vec(x, y, z);

			Assert::AreEqual(x, vec.x(), L"Error: Init x entry");
			Assert::AreEqual(y, vec.y(), L"Error: Init y entry");
			Assert::AreEqual(z, vec.z(), L"Error: Init z entry");
		}

		TEST_METHOD(Add)
		{

			Vec3 v(REAL_CONST(2.0), REAL_CONST(3.0), REAL_CONST(1.5));
			Vec3 u(REAL_CONST(4.0), REAL_CONST(-1.0), REAL_CONST(2.5));


			Vec3 z = v + u;

			Assert::AreEqual(REAL_CONST(6.0), z.x(), L"Error x");
			Assert::AreEqual(REAL_CONST(2.0), z.y(), L"Error y");
			Assert::AreEqual(REAL_CONST(4.0), z.z(), L"Error z");

			z += Vec3(REAL_CONST(1.0), REAL_CONST(0.2), REAL_CONST(-0.5));

			Assert::AreEqual(REAL_CONST(7.0), z.x(), L"Error 2 x");
			Assert::AreEqual(REAL_CONST(2.2), z.y(), L"Error 2 y");
			Assert::AreEqual(REAL_CONST(3.5), z.z(), L"Error 2 z");
		}

		TEST_METHOD(Subtract)
		{

			Vec3 v(REAL_CONST(2.0), REAL_CONST(3.0), REAL_CONST(1.5));
			Vec3 u(REAL_CONST(4.0), REAL_CONST(-1.0), REAL_CONST(2.5));


			Vec3 z = v - u;

			Assert::AreEqual(REAL_CONST(-2.0), z.x(), L"Error x");
			Assert::AreEqual(REAL_CONST(4.0), z.y(), L"Error y");
			Assert::AreEqual(REAL_CONST(-1.0), z.z(), L"Error z");

			z -= Vec3(REAL_CONST(1.0), REAL_CONST(0.2), REAL_CONST(-0.5));

			Assert::AreEqual(REAL_CONST(-3.0), z.x(), L"Error 2 x");
			Assert::AreEqual(REAL_CONST(3.8), z.y(), L"Error 2 y");
			Assert::AreEqual(REAL_CONST(-0.5), z.z(), L"Error 2 z");
		}

		TEST_METHOD(Multiply)
		{

			Vec3 v(REAL_CONST(2.0), REAL_CONST(3.0), REAL_CONST(1.5));
			Vec3 z = REAL_CONST(2.0) * v;

			Assert::AreEqual(REAL_CONST(4.0), z.x(), L"Error x");
			Assert::AreEqual(REAL_CONST(6.0), z.y(), L"Error y");
			Assert::AreEqual(REAL_CONST(3.0), z.z(), L"Error z");

			z *= 3.0;

			Assert::AreEqual(REAL_CONST(12.0), z.x(), L"Error 2 x");
			Assert::AreEqual(REAL_CONST(18.0), z.y(), L"Error 2 y");
			Assert::AreEqual(REAL_CONST(9.0), z.z(), L"Error 2 z");
		}

#if MATRIX_LIBRARY == CUSTOM_FLAG
		TEST_METHOD(Divide)
		{

			Vec3 v(REAL_CONST(2.0), REAL_CONST(3.0), REAL_CONST(4.0));
			Vec3 z = REAL_CONST(24.0) / v;

			Assert::AreEqual(REAL_CONST(12.0), z.x(), L"Error x");
			Assert::AreEqual(REAL_CONST(8.0), z.y(), L"Error y");
			Assert::AreEqual(REAL_CONST(6.0), z.z(), L"Error z");

			z /= 4.0;

			Assert::AreEqual(REAL_CONST(3.0), z.x(), L"Error 2 x");
			Assert::AreEqual(REAL_CONST(2.0), z.y(), L"Error 2 y");
			Assert::AreEqual(REAL_CONST(1.5), z.z(), L"Error 2 z");
		}
#endif

		TEST_METHOD(Negative)
		{
			const int a = 2;
			const int b = 3;

			Vec3 v(REAL_CONST(-2.0), REAL_CONST(3.0), REAL_CONST(1.5));
			Vec3 z = -v;

			Assert::AreEqual(REAL_CONST(2.0), z.x(), L"Error x");
			Assert::AreEqual(REAL_CONST(-3.0), z.y(), L"Error y");
			Assert::AreEqual(REAL_CONST(-1.5), z.z(), L"Error z");
		}

		TEST_METHOD(Comparison)
		{
			const int a = 2;
			const int b = 3;

			Real x = REAL_CONST(0.8);
			Real y = REAL_CONST(7.1);
			Real z = REAL_CONST(-0.2);
			Vec3 v(x, y, z);
			Vec3 u(x, y, z);

			Assert::AreEqual(true, v == u, L"Match");

			u.x() = REAL_CONST(1.0);
			v.y() = REAL_CONST(1.0);
			u.z() = REAL_CONST(1.0);

			Assert::AreEqual(false, v == u, L"No match");
		}

		TEST_METHOD(Normalise)
		{
			Vec3 vec(1.0, 2.0, 2.0);

			Real norm = vec.Normal();
			Assert::AreEqual(REAL_CONST(3.0), norm, L"Error norm");

			Vec3 normalVec = vec.Normalised();
			vec.Normalise();

			Assert::AreEqual((Real)(1.0 / 3.0), vec.x(), L"Error x");
			Assert::AreEqual((Real)(2.0 / 3.0), vec.y(), L"Error y");
			Assert::AreEqual((Real)(2.0 / 3.0), vec.z(), L"Error z");

			Assert::AreEqual(REAL_CONST(1.0), vec.Normal(), L"Error normalised vector longer than one");
			Assert::AreEqual(normalVec.x(), vec.x(), L"Error 2 x");
			Assert::AreEqual(normalVec.y(), vec.y(), L"Error 2 y");
			Assert::AreEqual(normalVec.z(), vec.z(), L"Error 2 z");
		}

		TEST_METHOD(Sum)
		{
			Vec3 vec(-1.0, 2.0, 2.5);

			Real sum = vec.Sum();
			Assert::AreEqual(REAL_CONST(3.5), sum, L"Error sum");
		}

		TEST_METHOD(Dot)
		{
			Vec3 v(REAL_CONST(-1.0), REAL_CONST(2.0), REAL_CONST(2.5));
			Vec3 u(REAL_CONST(3.0), REAL_CONST(-0.5), REAL_CONST(2.0));

			Real dot = v.dot(u);
			Assert::AreEqual(REAL_CONST(1.0), dot, L"Error dot");
		}

		TEST_METHOD(Cross)
		{
			Vec3 v(REAL_CONST(-1.0), REAL_CONST(2.0), REAL_CONST(2.5));
			Vec3 u(REAL_CONST(3.0), REAL_CONST(-0.5), REAL_CONST(2.0));

			Vec3 cross = v.cross(u);
			Assert::AreEqual(REAL_CONST(5.25), cross.x(), L"Error x");
			Assert::AreEqual(REAL_CONST(9.5), cross.y(), L"Error y");
			Assert::AreEqual(REAL_CONST(-5.5), cross.z(), L"Error z");
		}
	};
#pragma optimize("", on)
}