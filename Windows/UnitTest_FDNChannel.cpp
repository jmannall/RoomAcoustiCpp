
#include "CppUnitTest.h"
#define NOMINMAX
// #include <windows.h>

#include "UtilityFunctions.h"

#include "Spatialiser/FDN.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RAC
{
	using namespace Spatialiser;

#pragma optimize("", off)

	TEST_CLASS(FDNChannel_Class)
	{
	public:

		TEST_METHOD(DelayLine)
		{
			const int delay = 127;
			const Coefficients T60({ 0.1, 0.2, 0.3, 0.4, 0.4 });
			const std::shared_ptr<Config> config = std::make_shared<Config>();
			const Real lerpFactor = config->GetLerpFactor();

			FDNChannel channel(delay, T60, config);

			Assert::AreEqual(0.0, channel.GetOutput(RandomValue(), lerpFactor), L"Output not zero");
			for (int i = 1; i < delay; i++)
				Assert::AreEqual(0.0, channel.GetOutput(RandomValue(), lerpFactor), L"Output not zero");
			Assert::AreNotEqual(0.0, channel.GetOutput(RandomValue(), lerpFactor), L"Output zero");
		}

		TEST_METHOD(ResetAbsorption)
		{
			const int delay = 73;
			const Coefficients T60({ 0.1, 0.2, 0.3, 0.4, 0.4 });
			const Coefficients reflectionGains({ 0.87, 0.75, 0.81, 0.84, 0.84 });
			const std::shared_ptr<Config> config = std::make_shared<Config>();
			const Real lerpFactor = config->GetLerpFactor();

			FDNChannel channel(delay, T60, config);

			const int numFrames = 256;
			Buffer in(numFrames);
			Buffer out(numFrames);
			for (int i = 0; i < numFrames; i++)
				in[i] = channel.GetOutput(RandomValue(), lerpFactor);
			channel.Reset();

			Assert::AreEqual(0.0, channel.GetOutput(RandomValue(), lerpFactor), L"GetOutput not zero");
			for (int i = 1; i < delay; i++)
				Assert::AreEqual(0.0, channel.GetOutput(RandomValue(), lerpFactor), L"GetOutput not zero");
			Assert::AreNotEqual(0.0, channel.GetOutput(RandomValue(), lerpFactor), L"GetOutput zero");
		}

		TEST_METHOD(ResetReflection)
		{
			const int delay = 73;
			const Coefficients T60({ 0.1, 0.2, 0.3, 0.4, 0.4 });
			const Coefficients reflectionGains({ 0.87, 0.75, 0.81, 0.84, 0.84 });
			const std::shared_ptr<Config> config = std::make_shared<Config>();
			const Real lerpFactor = config->GetLerpFactor();

			FDNChannel channel(delay, T60, config);
			channel.SetTargetReflectionFilter(reflectionGains);

			const int numFrames = 256;
			Buffer in(numFrames);
			Buffer out(numFrames);
			for (int i = 0; i < numFrames; i++)
				in[i] = channel.GetOutput(RandomValue(), lerpFactor);
			channel.ProcessOutput(in, out, lerpFactor);
			channel.Reset();

			in.Reset();
			channel.ProcessOutput(in, out, lerpFactor);
			for (int i = 0; i < numFrames; i++)
				Assert::AreEqual(0.0, out[i], L"ProcessOutput not zero");
		}

		TEST_METHOD(ProcessReflection)
		{
			const int delay = 7;
			const Coefficients T60({ 0.1, 0.2, 0.3, 0.4, 0.4 });
			const Real target = 0.6;
			const Coefficients reflectionGains({ target, target, target, target, target });
			const std::shared_ptr<Config> config = std::make_shared<Config>();
			const Real lerpFactor = 1.0;

			FDNChannel channel(delay, T60, config);
			channel.SetTargetReflectionFilter(reflectionGains);

			const int numFrames = 256;
			Buffer in(numFrames);
			in[0] = 1.0;
			Buffer out(numFrames);

			channel.ProcessOutput(in, out, lerpFactor);
			in[0] = 1.0;
			channel.ProcessOutput(in, out, lerpFactor);
			Assert::AreEqual(target, out[0], 10e-16, L"Reflection filter incorrect");
		}

		TEST_METHOD(ProcessAbsorption)
		{
			const int delay = 750;
			const Coefficients T60({ 1.2, 1.2, 1.2, 1.2, 1.2 });
			const std::shared_ptr<Config> config = std::make_shared<Config>();
			// config->lerpFactor = 0.5;
			const Real lerpFactor = config->GetLerpFactor();

			const Real target = std::pow(10.0, (-60.0 * (static_cast<Real>(delay) / static_cast<Real>(config->fs)) / 20.0) / T60[0]);

			FDNChannel channel(delay, T60, config);

			channel.GetOutput(1.0, lerpFactor);
			for (int i = 1; i < delay; i++)
				channel.GetOutput(0.0, lerpFactor);
			Real out = channel.GetOutput(0.0, lerpFactor);
			Assert::AreEqual(target, out, 10e-16, L"Absorption filter incorrect");
		}

		TEST_METHOD(UpdateT60)
		{
			const int delay = 100;
			const Coefficients T60({ 0.2, 0.2, 0.2, 0.2, 0.2 });
			const Coefficients newT60({ 0.6, 0.6, 0.6, 0.6, 0.6 });
			const std::shared_ptr<Config> config = std::make_shared<Config>();
			const Real lerpFactor = 1.0;

			const Real target = std::pow(10.0, (-60.0 * (static_cast<Real>(delay) / static_cast<Real>(config->fs)) / 20.0) / newT60[0]);

			FDNChannel channel(delay, T60, config);
			channel.SetTargetT60(newT60);

			channel.GetOutput(0.0, lerpFactor);
			channel.GetOutput(1.0, lerpFactor);
			for (int i = 1; i < delay; i++)
				channel.GetOutput(0.0, lerpFactor);
			Real out = channel.GetOutput(0.0, lerpFactor);
			Assert::AreEqual(target, out, 10e-16, L"Absorption filter not interpolating");
		}
	};
#pragma optimize("", on)
}