
#ifndef UtilityFunctions_h
#define UtilityFunctions_h

#include <string>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <filesystem>
#include <random>

const std::string dir = _SOLUTIONDIR;
const std::string filePath = dir + "UnitTestData\\";

template <typename T>
inline std::string ToStr(T x) { return std::to_string(x); }

//////////////////// Parse csv files ////////////////////

template <typename T>
inline T StrTo(const std::string& str) {
    if constexpr (std::is_same_v<T, double>)
        return std::stod(str);
	else if constexpr (std::is_same_v<T, float>)
		return std::stof(str);
	else if constexpr (std::is_same_v<T, int>)
        return std::stoi(str);
	else if constexpr (std::is_same_v<T, std::string>)
		return str;
    else
        static_assert(std::is_same_v<T, double> || std::is_same_v<T, int>, "Unsupported type");
}

template <typename T>
inline std::vector<T> Parse1Dcsv(std::string filePath)
{
	std::ifstream  data(filePath);
	std::string line;
	std::vector<T> parsedCsv;
	while (std::getline(data, line))
	{
		std::stringstream lineStream(line);
		std::string cell;
		std::vector<T> parsedRow;
		std::getline(lineStream, cell, ',');
		parsedCsv.push_back(StrTo<T>(cell));
	}
	return parsedCsv;
}

template <typename T>
inline std::vector<std::vector<T>> Parse2Dcsv(std::string filePath)
{
    std::ifstream data(filePath);
    std::string line;
    std::vector<std::vector<T>> parsedCsv;
    while (std::getline(data, line))
    {
        std::stringstream lineStream(line);
        std::string cell;
        std::vector<T> parsedRow;
        while (std::getline(lineStream, cell, ','))
        {
            parsedRow.push_back(StrTo<T>(cell));
        }

        parsedCsv.push_back(parsedRow);
    }
    return parsedCsv;
}

inline void AppendBufferToCSV(const std::string& filename, const double* data, const int length) {
	// Open the file in append mode
	std::ofstream file(filename, std::ios::app);

	// Check if the file is open
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filename << std::endl;
		return;
	}

	int decimalPlaces = 18;

	// Set the decimal precision for floating point numbers
	file << std::fixed << std::setprecision(decimalPlaces);

	// Write the vector data to the file as a CSV row
	for (int i = 0; i < length; ++i) {
		file << data[i];
		// Add a comma after every element except the last one
		if (i != length - 1) {
			file << ",";
		}
	}
	// End the row by adding a newline
	file << "\n";

	// Close the file
	file.close();
}

inline void WriteDataEntry(std::string filename, const float* data, int length, double position, double rotation) {

	// Open the file in append mode
	std::ofstream file(filename, std::ios::app);

	// Check if the file is open
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filename << std::endl;
		return;
	}

	int pos = position * 100;
	int rot = ceil(rotation);
	file << pos;
	file << "_";
	file << rot;
	file << ",";

	// Write the vector data to the file as a CSV row
	for (int i = 0; i < length; ++i) {
		file << static_cast<int>(data[i] * 8388607.0);
		// Add a comma after every element except the last one
		if (i != length - 1) {
			file << ",";
		}
	}
	// End the row by adding a newline
	file << "\n";
	// Close the file
	file.close();
}

//////////////////// Directory info ////////////////////

inline std::vector<std::string> ListDirectoryFiles(const std::string& directoryPath)
{
	std::vector<std::string> files;

	try
	{
		for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
		{
			if (entry.is_regular_file())
				files.push_back(entry.path().string());  // Get full path
		}
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		std::cerr << "Error accessing directory: " << e.what() << std::endl;
	}

	return files;
}

static std::default_random_engine generator(100); // Seed the generator

inline double RandomValue()
{
	std::uniform_real_distribution<double> distribution(-1.0, 1.0);
	return distribution(generator);
}

inline double RandomValue(const double a, const double b)
{
	std::uniform_real_distribution<double> distribution(a, b);
	return distribution(generator);
}

#endif