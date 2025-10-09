
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "Common/Vec4.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Common;

#pragma optimize("", off)

	TEST_CLASS(Vec4_Class)
	{
	public:

		TEST_METHOD(Assign)
		{
			Real w = (Real)0.5;
			Real x = (Real)1.0;
			Real y = (Real)-2.0;
			Real z = (Real)3.0;
			Vec4 q;

			q.w() = w;
			q.x() = x;
			q.y() = y;
			q.z() = z;

			Assert::AreEqual(w, q.w(), L"Error: Add w entry");
			Assert::AreEqual(x, q.x(), L"Error: Add x entry");
			Assert::AreEqual(y, q.y(), L"Error: Add y entry");
			Assert::AreEqual(z, q.z(), L"Error: Add z entry");
		}

		TEST_METHOD(Init)
		{
			Real w = (Real)0.5;
			Real x = (Real)0.8;
			Real y = (Real)7.1;
			Real z = (Real)-0.2;
			Vec4 q(w, x, y, z);

			Assert::AreEqual(w, q.w(), L"Error: Init w entry");
			Assert::AreEqual(x, q.x(), L"Error: Init x entry");
			Assert::AreEqual(y, q.y(), L"Error: Init y entry");
			Assert::AreEqual(z, q.z(), L"Error: Init z entry");

			w = (Real)0.8;
			Vec3 vec(1.2, 3.4, -1.0);
			Vec4 q2(w, vec);

			Assert::AreEqual(w, q.x(), L"Error: Init 2 w entry");
			Assert::AreEqual(x, q.x(), L"Error: Init 2 x entry");
			Assert::AreEqual(y, q.y(), L"Error: Init 2 y entry");
			Assert::AreEqual(z, q.z(), L"Error: Init 2 z entry");
		}

#if MATRIX_LIBRARY == CUSTOM_FLAG
		TEST_METHOD(Divide)
		{
			Vec4 q((Real)6.0, (Real)3.0, (Real)4.0, (Real)-3.0);
			Vec4 z = q / (Real)2.0;

			Assert::AreEqual((Real)3.0, z.w(), (Real)1e-10, L"Error w");
			Assert::AreEqual((Real)1.5, z.x(), (Real)1e-10, L"Error x");
			Assert::AreEqual((Real)2.0, z.y(), (Real)1e-10, L"Error y");
			Assert::AreEqual((Real)-1.5, z.z(), (Real)1e-10, L"Error z");
		}
#endif

		TEST_METHOD(Multiply)
		{
			Vec4 x((Real)6.0, (Real)3.0, (Real)4.0, (Real)-3.0);
			Vec4 y((Real)2.0, (Real)-2.0, (Real)1.5, (Real)-2.2);

			Vec4 z = x * y;
			Assert::AreEqual((Real)5.4, z.w(), (Real)1e-10, L"Error w");
			Assert::AreEqual((Real)-10.3, z.x(), (Real)1e-10, L"Error x");
			Assert::AreEqual((Real)29.6, z.y(), (Real)1e-10, L"Error y");
			Assert::AreEqual((Real)-6.7, z.z(), (Real)1e-10, L"Error z");
		}

		TEST_METHOD(RotateVec)
		{
			const Real constant = std::sin(PI_1 / (Real)4.0);

			Vec4 q(1.0, 0.0, 0.0, 0.0);
			Vec3 vec(0.0, 0.0, 1.0);

			Vec3 newVec = RotateVector(vec, q);

			Assert::AreEqual((Real)0.0, newVec.x(), (Real)1e-10, L"Error x");
			Assert::AreEqual((Real)0.0, newVec.y(), (Real)1e-10, L"Error y");
			Assert::AreEqual((Real)1.0, newVec.z(), (Real)1e-10, L"Error z");

			q = Vec4(constant, 0.0, constant, 0.0);
			newVec = RotateVector(vec, q);

			Assert::AreEqual((Real)-1.0, newVec.x(), (Real)1e-10, L"Error 2 x");
			Assert::AreEqual((Real)0.0, newVec.y(), (Real)1e-10, L"Error 2 y");
			Assert::AreEqual((Real)0.0, newVec.z(), (Real)1e-10, L"Error 2 z");

			q = Vec4(constant, 0.0, -constant, 0.0);
			newVec = RotateVector(vec, q);

			Assert::AreEqual((Real)1.0, newVec.x(), (Real)1e-10, L"Error 3 x");
			Assert::AreEqual((Real)0.0, newVec.y(), (Real)1e-10, L"Error 3 y");
			Assert::AreEqual((Real)0.0, newVec.z(), (Real)1e-10, L"Error 3 z");

			q = Vec4(0.0, 0.0, 1.0, 0.0);
			newVec = RotateVector(vec, q);

			Assert::AreEqual((Real)0.0, newVec.x(), (Real)1e-10, L"Error 4 x");
			Assert::AreEqual((Real)0.0, newVec.y(), (Real)1e-10, L"Error 4 y");
			Assert::AreEqual((Real)-1.0, newVec.z(), (Real)1e-10, L"Error 4 z");

			q = Vec4(constant, -constant, 0.0, 0.0);
			newVec = RotateVector(vec, q);

			Assert::AreEqual((Real)0.0, newVec.x(), (Real)1e-10, L"Error 5 x");
			Assert::AreEqual((Real)-1.0, newVec.y(), (Real)1e-10, L"Error 5 y");
			Assert::AreEqual((Real)0.0, newVec.z(), (Real)1e-10, L"Error 5 z");

			q = Vec4(constant, constant, 0.0, 0.0);
			newVec = RotateVector(vec, q);

			Assert::AreEqual((Real)0.0, newVec.x(), (Real)1e-10, L"Error 6 x");
			Assert::AreEqual((Real)1.0, newVec.y(), (Real)1e-10, L"Error 6 y");
			Assert::AreEqual((Real)0.0, newVec.z(), (Real)1e-10, L"Error 6 z");

			q = Vec4(-0.14645, -0.35355, 0.85355, -0.35355);
			vec = Vec3(0.0, -constant, constant);
			newVec = RotateVector(vec, q);

			Assert::AreEqual(constant, newVec.x(), (Real)1e-10, L"Error 7 x");
			Assert::AreEqual(-constant, newVec.y(), (Real)1e-10, L"Error 7 y");
			Assert::AreEqual((Real)0.0, newVec.z(), (Real)1e-10, L"Error 7 z");

		}

		TEST_METHOD(Conjugate)
		{
			Vec4 q((Real)-2.0, (Real)3.0, (Real)1.5, (Real)-3.0);
			Vec4 conjQ = q.Conjugate();

			Assert::AreEqual((Real)-2.0, conjQ.w(), L"Error w");
			Assert::AreEqual((Real)-3.0, conjQ.x(), L"Error x");
			Assert::AreEqual((Real)-1.5, conjQ.y(), L"Error y");
			Assert::AreEqual((Real)3.0, conjQ.z(), L"Error z");
		}

		TEST_METHOD(Inverse)
		{
			Vec4 q((Real)2.0, (Real)-3.0, (Real)1.5, (Real)1.1);
			Vec4 inverseQ = q.InverseMatrix();

			Assert::AreEqual((Real)0.121506682867558, inverseQ.w(), (Real)1e-6, L"Error w");
			Assert::AreEqual((Real)0.182260024301337, inverseQ.x(), (Real)1e-6, L"Error x");
			Assert::AreEqual((Real)-0.091130012150668, inverseQ.y(), (Real)1e-6, L"Error y");
			Assert::AreEqual((Real)-0.066828675577157, inverseQ.z(), (Real)1e-6, L"Error z");
		}

		TEST_METHOD(Comparison)
		{

			Real w = (Real)0.5;
			Real x = (Real)0.8;
			Real y = (Real)7.1;
			Real z = (Real)-0.2;
			Vec4 v(w, x, y, z);
			Vec4 u(w, x, y, z);

			Assert::AreEqual(true, v == u, L"Match");

			u.w() = (Real)1.0;
			u.x() = (Real)1.0;
			v.y() = (Real)1.0;
			u.z() = (Real)1.0;

			Assert::AreEqual(false, v == u, L"No match");
		}

		TEST_METHOD(Dot)
		{
			Vec4 v((Real)-1.0, (Real)2.0, (Real)2.5, (Real)-1.2);
			Vec4 u((Real)3.0, (Real)-0.5, (Real)2.0, (Real)-2.0);

			Real dot = v.dot(u);
			Assert::AreEqual(3.4, dot, L"Error dot");
		}

		TEST_METHOD(ForwardVector)
		{
			Vec4 q((Real)-1.0, (Real)2.0, (Real)2.5, (Real)-1.2);

			Vec3 vec = Forward(q.Normalised());
			Assert::AreEqual((Real)-0.772261623325453, vec.x(), (Real)1e-6, L"Error x");
			Assert::AreEqual((Real)-0.157604412923562, vec.y(), (Real)1e-6, L"Error y");
			Assert::AreEqual((Real)-0.615445232466509, vec.z(), (Real)1e-6, L"Error z");
		}
	};
#pragma optimize("", on)
}