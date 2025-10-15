/*
* @brief Utility to parse command line arguments
*
*/

#pragma once
#include <functional>
#include <map>
#include <string>
#include "ProfilePlan.h"

class CommandLineParser
{
public:
	CommandLineParser(int argc, const char *argv[]);

	bool Parse();

	void RegisterProfileTest(const std::string &name, const ProfileFunction &test);

	bool GetDebugFlag() const { return debugFlag;  }
	bool GetDetailedLogs() const { return detailedLogs; }
	int GetIterations() const { return iterations; }
	const std::string &GetLogPrefix() const { return logPrefix; }
	const std::string &GetProfileDataDirectory() const { return profileDataDirectory; }
	const std::vector<ProfilePlan> &GetPlan() const { return plan; }

private:
	void ShowHelp() const;

	static bool ParseStandardArgument(const std::string &argument, const std::string& prefix, std::string &outValue);
	static std::string GetTimestamp();

	// arguments
	int argumentCount;
	const char** arguments;

	// registered tests
	std::map<std::string, ProfileFunction> registeredTests;

	// resulting values
	int iterations = 3;
	bool debugFlag = false;
	bool detailedLogs = false;
	std::string logPrefix;
	std::string profileDataDirectory;
	std::vector<ProfilePlan> plan;
};
