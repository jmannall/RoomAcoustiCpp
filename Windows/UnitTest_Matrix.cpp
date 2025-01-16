
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "Common/Matrix.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Common;

#pragma optimize("", off)

	TEST_CLASS(Matrix_Class)
	{
	public:

		TEST_METHOD(Assign)
		{
			const int rows = 5;
			const int cols = 4;

			Matrix m = Matrix(rows, cols);

			Real x = 1.0;

			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					m[i][j] = x;
					Assert::AreEqual(x, m[i][j], L"Error: Add entry");
				}
			}
		}

		TEST_METHOD(Init)
		{
			const int rows = 5;
			const int cols = 4;

			Matrix m = Matrix(rows, cols);

			Real x = 1.0;

			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					m[i][j] = x;
					Assert::AreEqual(x, m[i][j], L"Error: Add entry");
					m[i][j] += 1.0;
					x += 1.0;
					Assert::AreEqual(x, m[i][j], L"Error: Increase entry");
				}
			}

			Matrix mat = Matrix(m.Data());

			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					Assert::AreEqual(m[i][j], mat[i][j], L"Error: Init from vectors");
				}
			}

			m.Reset();

			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					Real test = m[i][j];
					Assert::AreEqual(0.0, test, L"Error: Reset");
				}
			}
		}

		TEST_METHOD(Multiply)
		{
			const int a = 2;
			const int b = 3;

			Matrix x = Matrix(a, b);
			Matrix y = Matrix(b, a);

			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < b; j++)
				{
					x[i][j] = 1.0;
					y[j][i] = 1.0;
				}
			}

			x[0][0] = 2.0;
			x[0][2] = 5.0;
			x[1][1] = 3.0;
			y[1][0] = 4.0;
			y[0][1] = 3.0;

			Matrix z = x * y;

			Assert::AreEqual(11.0, z[0][0], L"Error (0, 0)");
			Assert::AreEqual(12.0, z[0][1], L"Error (0, 1)");
			Assert::AreEqual(14.0, z[1][0], L"Error (1, 0)");
			Assert::AreEqual(7.0, z[1][1], L"Error (1, 1)");

			z *= 2.0;

			Assert::AreEqual(22.0, z[0][0], L"Error 2 (0, 0)");
			Assert::AreEqual(24.0, z[0][1], L"Error 2 (0, 1)");
			Assert::AreEqual(28.0, z[1][0], L"Error 2 (1, 0)");
			Assert::AreEqual(14.0, z[1][1], L"Error 2 (1, 1)");

			Matrix w = z * 2.0;

			Assert::AreEqual(44.0, w[0][0], L"Error 3 (0, 0)");
			Assert::AreEqual(48.0, w[0][1], L"Error 3 (0, 1)");
			Assert::AreEqual(56.0, w[1][0], L"Error 3 (1, 0)");
			Assert::AreEqual(28.0, w[1][1], L"Error 3 (1, 1)");
		}

		TEST_METHOD(Add)
		{
			const int a = 2;
			const int b = 3;

			Matrix x = Matrix(a, b);
			Matrix y = Matrix(a, b);

			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < b; j++)
				{
					x[i][j] = 1.0;
					y[i][j] = 1.0;
				}
			}

			x[0][0] = 2.0;
			x[0][2] = 5.0;
			x[1][1] = 3.0;
			y[1][0] = 4.0;
			y[0][1] = 3.0;
			y[0][0] = 7.0;

			Matrix z = x + y;

			Assert::AreEqual(9.0, z[0][0], L"Error (0, 0)");
			Assert::AreEqual(4.0, z[0][1], L"Error (0, 1)");
			Assert::AreEqual(6.0, z[0][2], L"Error (0, 2)");
			Assert::AreEqual(5.0, z[1][0], L"Error (1, 0)");
			Assert::AreEqual(4.0, z[1][1], L"Error (1, 1)");
			Assert::AreEqual(2.0, z[1][2], L"Error (1, 2)");
		}

		TEST_METHOD(Negative)
		{
			const int a = 2;
			const int b = 3;

			Matrix x = Matrix(a, b);

			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < b; j++)
				{
					x[i][j] = 1.0;
				}
			}

			Matrix y = -x;

			Assert::AreEqual(-1.0, y[0][0], L"Error (0, 0)");
			Assert::AreEqual(-1.0, y[0][1], L"Error (0, 1)");
			Assert::AreEqual(-1.0, y[0][2], L"Error (0, 2)");
			Assert::AreEqual(-1.0, y[1][0], L"Error (1, 0)");
			Assert::AreEqual(-1.0, y[1][1], L"Error (1, 1)");
			Assert::AreEqual(-1.0, y[1][2], L"Error (1, 2)");
		}

		TEST_METHOD(Comparison)
		{
			const int a = 2;
			const int b = 3;

			Matrix x = Matrix(a, b);
			Matrix y = Matrix(a, b);

			Assert::AreEqual(true, x == y, L"Match");

			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < b; j++)
				{
					x[i][j] = 1.0;
				}
			}

			Assert::AreEqual(false, x == y, L"No match");
		}
	};
#pragma optimize("", on)
}