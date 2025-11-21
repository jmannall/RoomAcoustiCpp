/*
* @brief Utility to parse command line arguments
*
*/

#include "CommandLineParser.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <ranges>

CommandLineParser::CommandLineParser(int argc, const char *argv[])
	: argumentCount(argc), arguments(argv)
{
	logPrefix = GetTimestamp() + "_RoomAcoustiCpp";
}

bool CommandLineParser::Parse()
{
	for (int argumentIndex = 1; argumentIndex < argumentCount; ++argumentIndex)
	{
		std::string argument(arguments[argumentIndex]);
		std::string value;

		if (argument == "--help")
		{
			ShowHelp();
			return false;
		}
		else if (argument == "--debug")
		{
			debugFlag = true;
		}
		else if (argument == "--detailed-logs")
		{
			detailedLogs = true;
		}
		else if (argument == "--no-debug")
		{
			debugFlag = false;
		}
		else if (argument == "--no-detailed-logs")
		{
			detailedLogs = false;
		}
		else if (ParseStandardArgument(argument, "--test-iterations=", value))
		{
			const int newIterations = std::stoi(value);
			if (newIterations < 0)
			{
				std::cerr << "Invalid test-iterations: " << argument << std::endl;
				return false;
			}
			testIterations = newIterations;
		}
		else if (ParseStandardArgument(argument, "--num-rays=", value))
		{
			const int newNumRays = std::stoi(value);
			if (newNumRays <= 0)
			{
				std::cerr << "Invalid num-rays: " << argument << std::endl;
				return false;
			}
			numRays = newNumRays;
		}
		else if (ParseStandardArgument(argument, "--reflection-order=", value))
		{
			const int newReflectionOrder = std::stoi(value);
			if (newReflectionOrder <= 0)
			{
				std::cerr << "Invalid reflection-order: " << argument << std::endl;
				return false;
			}
			reflectionOrder = newReflectionOrder;
		}
		else if (ParseStandardArgument(argument, "--shadow-order=", value))
		{
			const int newShadowOrder = std::stoi(value);
			if (newShadowOrder <= 0)
			{
				std::cerr << "Invalid shadoiw-order: " << argument << std::endl;
				return false;
			}
			shadowOrder = newShadowOrder;
		}
		else if (argument == "--dynamic-scene")
		{
			staticScene = false;
		}
		else if (ParseStandardArgument(argument, "--inner-iterations=", value))
		{
			const int newIterations = std::stoi(value);
			if (newIterations < 0)
			{
				std::cerr << "Invalid inner-iterations: " << argument << std::endl;
				return false;
			}
			innerIterations = newIterations;
		}
		else if (ParseStandardArgument(argument, "--audio-threads=", value))
		{
			const int newDesiredAudioThreads = std::stoi(value);
			if (newDesiredAudioThreads < 0)
			{
				std::cerr << "Invalid audio-threads: " << argument << std::endl;
				return false;
			}
			desiredAudioThreads = newDesiredAudioThreads;
		}
		else if (ParseStandardArgument(argument, "--log-prefix=", value))
		{
			logPrefix = value;
		}
		else if (ParseStandardArgument(argument, "--profile-data=", value))
		{
			profileDataDirectory = value;
		}
		else if (argument.starts_with("-"))
		{
			std::cerr << "Invalid argument: " << argument << std::endl;
			ShowHelp();
			return false;
		}
		else
		{
			const auto test = registeredTests.find(argument);
			if (test == registeredTests.end())
			{
				std::cerr << "Invalid test: " << argument << std::endl;
				return false;
			}
			plan.push_back({ test->first, test->second });
		}
	}

	// if the user didn't specify any tests, then just add them all
	if (plan.empty())
	{
		for (const auto &registeredTest : registeredTests)
			plan.push_back({ registeredTest.first, registeredTest.second });
	}
	return true;
}

void CommandLineParser::RegisterProfileTest(const std::string &name, const ProfileFunction &test)
{
	registeredTests.emplace(name, test);
}

void CommandLineParser::ShowHelp() const
{
	// print basic usage
	std::cout << arguments[0] << R"( [options] test1,test2 ...

Options:
    --audio-threads=##	   Overrides the number of audio threads
    --debug                Enables certain memory debugging features
    --detailed-logs        Enables detailed logs
    --dynamic-scene        Moves the sources around a 1m^2 area
    --inner-iterations=##  The number of times to run the inner loop
    --log-prefix=file      Specifies the log prefix
    --no-debug             Disables certain memory debugging features
    --no-detailed-logs     Disabled detailed logs 
    --num-rays=##          Sets the number of rays
    --profile-data=xx      Sets the profile data directory.
    --reflection-order=##  Sets the reflection order
    --run-log=file         Specifies the log of each run
    --shadow-order=##      Sets the shadow order
    --test-iterations=##   The number of times to run each test

If the list of tests isn't specified, it will default to running all tests.  If a profile data directory
isn't specified, then it will search for it.

Supported tests:
      )";

	// find all registered tests and print them
	std::vector<std::string> testNames;
	for (const auto &key : registeredTests | std::views::keys)
		testNames.push_back(key);
	std::ranges::sort(testNames);
	for (int testNameIndex = 0; testNameIndex < testNames.size(); ++testNameIndex)
	{
		if (testNameIndex > 0)
			std::cout << ", ";
		std::cout << testNames[testNameIndex];
	}
	std::cout << std::endl;
}

bool CommandLineParser::ParseStandardArgument(const std::string &argument, const std::string& prefix, std::string &outValue)
{
	if (!argument.starts_with(prefix))
		return false;
	outValue = argument.substr(prefix.length());
	return true;
}

std::string CommandLineParser::GetTimestamp()
{
	// Get current time
	auto now = std::chrono::system_clock::now();
	std::time_t time_now = std::chrono::system_clock::to_time_t(now);

	std::tm local_time;
	localtime_s(&local_time, &time_now);

	// Format time into string: YYYY-MM-DD_HH-MM-SS
	std::stringstream ss;
	ss << std::put_time(&local_time, "%Y-%m-%d_%H-%M-%S");
	std::string timestamp = ss.str();

	// Full log file path
	return timestamp;
}

