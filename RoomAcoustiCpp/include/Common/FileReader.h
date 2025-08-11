
#include<vector>
#include <string>

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
            parsedRow.push_back(StrTo<T>(cell));

        parsedCsv.push_back(parsedRow);
    }
    return parsedCsv;
}

template <typename T>
inline std::vector<T> Parse1Dcsv(std::string filePath)
{
    return Parse2Dcsv(filePath)[0];
}

