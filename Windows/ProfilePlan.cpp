#include "ProfilePlan.h"

#include <cassert>
#include <framework.h>
#include <iostream>

SimpleTimer SimpleTimer::GetTickCount()
{
	LARGE_INTEGER  time = { 0 };
	QueryPerformanceCounter(&time);
	return SimpleTimer(time);
}

double SimpleTimer::GetMilliseconds(const SimpleTimer &a, const SimpleTimer &b)
{
	// make sure we have the resolution
	static bool gotResolution = false;
	static LARGE_INTEGER performanceFrequency;
	if (!gotResolution)
	{
		if (!QueryPerformanceFrequency(&performanceFrequency))
			return 0.0;
		gotResolution = true;
	}

	// find the difference
	const auto timeDiff = b.time.QuadPart - a.time.QuadPart;
	return static_cast<double>(timeDiff) * 1000.0 / static_cast<double>(performanceFrequency.QuadPart);
}

void ProfileExecutionContext::SetExecutionStage(ProfileExecutionStage stage)
{
	static const char *names[] = 
	{
		"Init",
		"Main",
		"Exit"
	};
	assert(static_cast<int>(stage) >= 0 && stage < ProfileExecutionStage::COUNT);

	std::cout << "Entering stage: " << names[static_cast<int>(stage)] << std::endl;

	const auto currentTime = SimpleTimer::GetCurrentTime();
	for (int index = static_cast<int>(stage); index < (int)ProfileExecutionStage::COUNT; ++index)
		stageTimers[index] = currentTime;
}

void ProfileExecutionContext::CompleteExecution()
{
	const auto now = SimpleTimer::GetCurrentTime();
	TotalTime += SimpleTimer::GetMilliseconds(stageTimers[(int)ProfileExecutionStage::Init], now);
	InitTime += SimpleTimer::GetMilliseconds(stageTimers[(int)ProfileExecutionStage::Init], stageTimers[(int)ProfileExecutionStage::Main]);
	MainTime += SimpleTimer::GetMilliseconds(stageTimers[(int)ProfileExecutionStage::Main], stageTimers[(int)ProfileExecutionStage::Exit]);
	ExitTime += SimpleTimer::GetMilliseconds(stageTimers[(int)ProfileExecutionStage::Exit], now);
}

