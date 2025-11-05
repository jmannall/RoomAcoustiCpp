/*
* @brief Structures related to running profile tests.
*
*/

#pragma once

#include <framework.h>
#include <functional>
#include <optional>
#include <string>

class SimpleTimer
{
public:
	SimpleTimer() = default;

	static SimpleTimer GetCurrentTime();
	static double GetMilliseconds(const SimpleTimer &a, const SimpleTimer &b);

private:
	SimpleTimer(LARGE_INTEGER theTime) : time(theTime) {}

	LARGE_INTEGER time;
};

enum class ProfileExecutionStage : int
{
	Init,
	Main,
	Exit,

	COUNT
};

struct ProfileExecutionContext
{
	std::string name;
	std::string logPrefix;
	int currentTestIteration;
	int totalTestIterations;
	int innerIterations;
	int numRays = 100;
	int reflectionOrder = 2;
	int shadowOrder = 2;
	bool staticScene = true;
	std::optional<size_t> desiredAudioThreads;

	SimpleTimer stageTimers[(int)ProfileExecutionStage::COUNT];

	double TotalTime = 0.0;
	double InitTime = 0.0;
	double MainTime = 0.0;
	double ExitTime = 0.0;

	void SetExecutionStage(ProfileExecutionStage stage);
	void CompleteExecution();
};

typedef std::function<void(ProfileExecutionContext &)> ProfileFunction;

struct ProfilePlan
{
	std::string name;
	ProfileFunction function;
};

