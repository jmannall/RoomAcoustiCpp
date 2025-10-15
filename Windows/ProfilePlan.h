/*
* @brief Structures related to running profile tests.
*
*/

#pragma once

struct ProfileExecutionContext
{
	std::string name;
	std::string logPrefix;
	int currentIteration;
	int totalIterations;
};

typedef std::function<void(const ProfileExecutionContext &)> ProfileFunction;

struct ProfilePlan
{
	std::string name;
	ProfileFunction function;
};

