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
		else if (ParseStandardArgument(argument, "--iterations=", value))
		{
			const int newIterations = std::stoi(value);
			if (newIterations < 0)
			{
				std::cerr << "Invalid iterations: " << argument << std::endl;
				return false;
			}
			iterations = newIterations;
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
    --debug             Enables certain memory debugging features
    --detailed-logs     Enables detailed logs
    --log-prefix=file   Specifies the log prefix
	--iterations=##     The number of times to run each test
    --no-debug          Disables certain memory debugging features
    --no-detailed-logs  Disabled detailed logs 
    --profile-data=xx   Sets the profile data directory.
    --run-log=file      Specifies the log of each run

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

