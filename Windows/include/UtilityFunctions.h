
#ifndef UtilityFunctions_h
#define UtilityFunctions_h

#include <string>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>

const std::string dir = _SOLUTIONDIR;
const std::string filePath = dir + "UnitTestData\\";

inline std::string IntToStr(int x)
{
	std::stringstream ss;
	ss << x;
	return ss.str();
}

inline std::string DoubleToStr(double x)
{
	std::stringstream ss;
	ss << x;
	return ss.str();
}

//////////////////// Parse csv files ////////////////////

inline double StrToDouble(const std::string& str) { return std::stod(str); }

inline std::vector<double> Parse1Dcsv(std::string filePath)
{
	std::ifstream  data(filePath);
	std::string line;
	std::vector<double> parsedCsv;
	while (std::getline(data, line))
	{
		std::stringstream lineStream(line);
		std::string cell;
		std::vector<double> parsedRow;
		std::getline(lineStream, cell, ',');
		parsedCsv.push_back(StrToDouble(cell));
	}
	return parsedCsv;
}

inline extern std::vector<std::vector<double> > Parse2Dcsv(std::string filePath)
{
	std::ifstream  data(filePath);
	std::string line;
	std::vector<std::vector<double> > parsedCsv;
	while (std::getline(data, line))
	{
		std::stringstream lineStream(line);
		std::string cell;
		std::vector<double> parsedRow;
		while (std::getline(lineStream, cell, ','))
		{
			parsedRow.push_back(StrToDouble(cell));
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

#endif