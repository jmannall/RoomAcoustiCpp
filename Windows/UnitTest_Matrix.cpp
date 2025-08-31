
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
					m(i, j) = x;
					Assert::AreEqual(x, m(i, j), L"Error: Add entry");
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
					m(i, j) = x;
					Assert::AreEqual(x, m(i, j), L"Error: Add entry");
					m(i, j) += 1.0;
					x += 1.0;
					Assert::AreEqual(x, m(i, j), L"Error: Increase entry");
				}
			}

			Matrix mat = Matrix(m);

			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					Assert::AreEqual(m(i, j), mat(i, j), L"Error: Init from matrix");
				}
			}

			m.Reset();

			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					Real test = m(i, j);
					Assert::AreEqual((Real)0.0, test, L"Error: Reset");
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
					x(i, j) = 1.0;
					y(j, i) = 1.0;
				}
			}

			x(0, 0) = 2.0;
			x(0, 2) = 5.0;
			x(1, 1) = 3.0;
			y(1, 0) = 4.0;
			y(0, 1) = 3.0;

			Matrix z = x * y;

			Assert::AreEqual((Real)11.0, z(0, 0), L"Error (0, 0)");
			Assert::AreEqual((Real)12.0, z(0, 1), L"Error (0, 1)");
			Assert::AreEqual((Real)14.0, z(1, 0), L"Error (1, 0)");
			Assert::AreEqual((Real)7.0, z(1, 1), L"Error (1, 1)");

			z *= 2.0;

			Assert::AreEqual((Real)22.0, z(0, 0), L"Error 2 (0, 0)");
			Assert::AreEqual((Real)24.0, z(0, 1), L"Error 2 (0, 1)");
			Assert::AreEqual((Real)28.0, z(1, 0), L"Error 2 (1, 0)");
			Assert::AreEqual((Real)14.0, z(1, 1), L"Error 2 (1, 1)");

			Matrix w = z * 2.0;

			Assert::AreEqual((Real)44.0, w(0, 0), L"Error 3 (0, 0)");
			Assert::AreEqual((Real)48.0, w(0, 1), L"Error 3 (0, 1)");
			Assert::AreEqual((Real)56.0, w(1, 0), L"Error 3 (1, 0)");
			Assert::AreEqual((Real)28.0, w(1, 1), L"Error 3 (1, 1)");
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
					x(i, j) = 1.0;
					y(i, j) = 1.0;
				}
			}

			x(0, 0) = 2.0;
			x(0, 2) = 5.0;
			x(1, 1) = 3.0;
			y(1, 0) = 4.0;
			y(0, 1) = 3.0;
			y(0, 0) = 7.0;

			Matrix z = x + y;

			Assert::AreEqual((Real)9.0, z(0, 0), L"Error (0, 0)");
			Assert::AreEqual((Real)4.0, z(0, 1), L"Error (0, 1)");
			Assert::AreEqual((Real)6.0, z(0, 2), L"Error (0, 2)");
			Assert::AreEqual((Real)5.0, z(1, 0), L"Error (1, 0)");
			Assert::AreEqual((Real)4.0, z(1, 1), L"Error (1, 1)");
			Assert::AreEqual((Real)2.0, z(1, 2), L"Error (1, 2)");
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
					x(i, j) = 1.0;
				}
			}

			Matrix y = -x;

			Assert::AreEqual((Real)-1.0, y(0, 0), L"Error (0, 0)");
			Assert::AreEqual((Real)-1.0, y(0, 1), L"Error (0, 1)");
			Assert::AreEqual((Real)-1.0, y(0, 2), L"Error (0, 2)");
			Assert::AreEqual((Real)-1.0, y(1, 0), L"Error (1, 0)");
			Assert::AreEqual((Real)-1.0, y(1, 1), L"Error (1, 1)");
			Assert::AreEqual((Real)-1.0, y(1, 2), L"Error (1, 2)");
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
					x(i, j) = 1.0;
				}
			}

			Assert::AreEqual(false, x == y, L"No match");
		}

		TEST_METHOD(Max)
		{
			const int a = 2;
			const int b = 3;

			std::vector<std::vector<Real>> mat = { {1.0, -2.0, 3.0}, {4.0, 5.0, -6.0} };
			Matrix x = Matrix(a, b);

			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < b; j++)
					x(i, j) = mat[i][j];
			}

			Real minValue = 1.9;
			x.Max(minValue);

			Assert::AreEqual(minValue, x(0, 0), L"Sample [0][0] incorrect");
			Assert::AreEqual(minValue, x(0, 1), L"Sample [0][1] incorrect");
			Assert::AreEqual(mat[0][2], x(0, 2), L"Sample [0][2] incorrect");
			Assert::AreEqual(mat[1][0], x(1, 0), L"Sample [1][0] incorrect");
			Assert::AreEqual(mat[1][1], x(1, 1), L"Sample [1][1] incorrect");
			Assert::AreEqual(minValue, x(1, 2), L"Sample [1][2] incorrect");
		}

		TEST_METHOD(Min)
		{
			const int a = 2;
			const int b = 3;

			std::vector<std::vector<Real>> mat = { {1.0, -2.0, 3.0}, {4.0, 5.0, -6.0} };
			Matrix x = Matrix(a, b);

			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < b; j++)
					x(i, j) = mat[i][j];
			}

			Real maxValue = 2.7;
			x.Min(maxValue);

			Assert::AreEqual(mat[0][0], x(0, 0), L"Sample [0][0] incorrect");
			Assert::AreEqual(mat[0][1], x(0, 1), L"Sample [0][1] incorrect");
			Assert::AreEqual(maxValue, x(0, 2), L"Sample [0][2] incorrect");
			Assert::AreEqual(maxValue, x(1, 0), L"Sample [1][0] incorrect");
			Assert::AreEqual(maxValue, x(1, 1), L"Sample [1][1] incorrect");
			Assert::AreEqual(mat[1][2], x(1, 2), L"Sample [1][2] incorrect");
		}

		TEST_METHOD(Pow10)
		{
			const int a = 2;
			const int b = 3;

			std::vector<std::vector<Real>> mat = { {1.0, -2.0, 3.0}, {4.0, 5.0, -6.0} };
			Matrix x = Matrix(a, b);

			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < b; j++)
					x(i, j) = mat[i][j];
			}

			std::vector<std::vector<Real>> result = { {10.0, 0.01, 1000.0}, {10000.0, 100000.0, 0.000001} };

			x.Pow10();

			Assert::AreEqual(result[0][0], x(0, 0), 1e-9, L"Sample [0][0] incorrect");
			Assert::AreEqual(result[0][1], x(0, 1), 1e-9, L"Sample [0][1] incorrect");
			Assert::AreEqual(result[0][2], x(0, 2), 1e-9, L"Sample [0][2] incorrect");
			Assert::AreEqual(result[1][0], x(1, 0), 1e-9, L"Sample [1][0] incorrect");
			Assert::AreEqual(result[1][1], x(1, 1), 1e-9, L"Sample [1][1] incorrect");
			Assert::AreEqual(result[1][2], x(1, 2), 1e-9, L"Sample [1][2] incorrect");
		}

		TEST_METHOD(Log10)
		{
			const int a = 2;
			const int b = 3;

			std::vector<std::vector<Real>> mat = { {1.0, 0.5, 3.0}, {4.0, 5.0, 0.1} };
			Matrix x = Matrix(a, b);

			for (int i = 0; i < a; i++)
			{
				for (int j = 0; j < b; j++)
					x(i, j) = mat[i][j];
			}

			std::vector<std::vector<Real>> result = { {0.0, -0.3010299957, 0.4771212547}, {0.6020599913, 0.6989700043, -1.0} };

			x.Log10();

			Assert::AreEqual(result[0][0], x(0, 0), 1e-10, L"Sample [0][0] incorrect");
			Assert::AreEqual(result[0][1], x(0, 1), 1e-10, L"Sample [0][1] incorrect");
			Assert::AreEqual(result[0][2], x(0, 2), 1e-10, L"Sample [0][2] incorrect");
			Assert::AreEqual(result[1][0], x(1, 0), 1e-10, L"Sample [1][0] incorrect");
			Assert::AreEqual(result[1][1], x(1, 1), 1e-10, L"Sample [1][1] incorrect");
			Assert::AreEqual(result[1][2], x(1, 2), 1e-10, L"Sample [1][2] incorrect");
		}

		TEST_METHOD(Inverse)
		{
			const int size = 5;
			std::vector<std::vector<Real>> mat = { {1.1, 1.2, -1.0, 0.7, 0.1}, {1.3, 2.1, 2.4, -0.9, -1.0}, {-2.1, 3.1, 1.6, 1.2, 3.5}, {1.2, -3.0, -0.12, 0.5, -0.2}, {1.2, -1.4, 1.2, -0.1, 2.3} };
			Matrix x = Matrix(size, size);

			for (int i = 0; i < size; i++)
			{
				for (int j = 0; j < size; j++)
					x(i, j) = mat[i][j];
			}

			std::vector<std::vector<Real>> result = { {0.34136477504077191545, 0.11496550315655131328, -0.10421070659136909674, 0.01819425113524224583, 0.19530667345622365649},
				{0.18262410121510679726, 0.066179659185312429936, 0.018120165280602643948, -0.18459725447700819591, -0.022792513179873540322},
				{-0.22633074246867897023, 0.27687658852419989748, 0.18414116343438314645, 0.3493796297485328533, -0.11961238404328984678},
				{0.24690160608254744694, 0.12410708023695619754, 0.39766724634732041366, 0.8252978992215058399, -0.49015602727939949022},
				{0.061879592705605533348, -0.15876012136250801363, -0.013383300949583094259, -0.26825870582877869972, 0.36010510066292022744} };

			x.Inverse();
			for (int i = 0; i < size; i++)
			{
				for (int j = 0; j < size; j++)
					Assert::AreEqual(result[i][j], x(i, j), 1e-15, L"Inverse incorrect");
			}
		}
	};
#pragma optimize("", on)
}