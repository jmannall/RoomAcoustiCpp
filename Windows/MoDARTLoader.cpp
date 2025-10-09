/*
* @brief Utility functions to load MoDART preprocessed data from files.
*
*/

#include "MoDARTLoader.h"

////////////////////////////////////////

bool ParseObjFile(const std::string& objFilePath, std::vector<Vec3>& outVertices, std::vector<Face>& outFaces)
{
    std::ifstream file(objFilePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open OBJ file: " << objFilePath << std::endl;
        return false;
    }

    std::string line;
    int currentPatch = 0;
    std::string currentMaterialName;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        if (prefix == "v")
        {
            float x, y, z;
            iss >> x >> y >> z;
            outVertices.emplace_back(x, y, z);
        }
        else if (prefix == "f")
        {
            Face face;
            std::string v1, v2, v3;
            iss >> v1 >> v2 >> v3;
            face.v[0] = std::stoi(v1) - 1;
            face.v[1] = std::stoi(v2) - 1;
            face.v[2] = std::stoi(v3) - 1;
            face.patchIndex = currentPatch;
            face.materialName = currentMaterialName;
            outFaces.push_back(face);
        }
        else if (prefix == "usemtl")
        {
            std::string matName;
            iss >> matName;
            // Parse node/material index from material name if possible
            std::smatch match;
            if (std::regex_search(matName, match, std::regex(R"(Patch_(\d+)_Mat_(.+))")))
            {
                currentPatch = std::stoi(match[1]);
                currentMaterialName = match[2];
            }
        }
    }
    return true;
}

////////////////////////////////////////

int BestBandMatch(float targetBand, Coefficients<> referenceBands)
{
    if (targetBand < referenceBands[0])
        return 0;
    else if (targetBand > referenceBands[referenceBands.Length() - 1])
        return referenceBands.Length() - 1;
    else
    {
        Real diff;
        int closestIndex = 0;
        Real smallestDiff = std::abs(targetBand - referenceBands[0]);

        for (int j = 1; j < referenceBands.Length(); j++)
        {
            diff = std::abs(targetBand - referenceBands[j]);
            if (diff < smallestDiff)
            {
                closestIndex = j;
                smallestDiff = diff;
            }
        }
        return closestIndex;
    }
}

Coefficients<> ResizeCoeffs(Coefficients<> targetFreqs, Coefficients<> inputFreqs, Coefficients<> inputCoeffs)
{
    Coefficients<> resizedCoeffs(targetFreqs.Length());

    if (inputFreqs.Length() != inputCoeffs.Length() || inputFreqs.Length() < 1)
    {
        std::cerr << "The arguments of ResizeCoeffs must have the same size and contain at least one element each." << std::endl;
        return resizedCoeffs;
    }

    for (int i = 0; i < targetFreqs.Length(); i++)
        resizedCoeffs[i] = inputCoeffs[BestBandMatch(targetFreqs[i], inputFreqs)];

    return resizedCoeffs;
}

////////////////////////////////////////

bool LoadMaterialsFromCsv(const std::string& mtlFilePath, Coefficients<>& outFrequencies, std::unordered_map<std::string, Coefficients<>>& outAbsorption, std::unordered_map<std::string, Coefficients<>>& outScattering)
{
    std::ifstream file(mtlFilePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open materials csv file: " << mtlFilePath << std::endl;
        return false;
    }

    std::string line;
    std::getline(file, line);
    std::istringstream iss(line);
    std::string token;

    std::getline(iss, token, ','); // Skip header

    std::vector<Real> freqBands;
    while (std::getline(iss, token, ','))
        freqBands.push_back(std::stof(token));
    int numFreqBands = SizeToInt(freqBands.size());

    if (numFreqBands <= 0)
    {
        std::cerr << "Invalid number of frequency bands in materials csv file." << std::endl;
        return false;
    }
    outFrequencies = Coefficients<>(freqBands);

    outAbsorption.clear();
    outScattering.clear();

    while (std::getline(file, line))
    {
        std::string materialName;
        std::vector<Real> absorption(numFreqBands);
        std::vector<Real> scattering(numFreqBands);

        // Read absorption line
        std::istringstream absIss(line);
        if (!std::getline(absIss, materialName, ','))
        {
            std::cerr << "Unexpected end of file while reading absorption data." << std::endl;
            return false;
        }
        for (int j = 0; j < numFreqBands; ++j)
        {
            if (!std::getline(absIss, token, ','))
            {
                std::cerr << "Insufficient absorption data for material " << materialName << "." << std::endl;
                return false;
            }
            absorption[j] = std::stof(token);
        }
        outAbsorption.emplace(materialName, absorption);

        // Read scattering line
        if (!std::getline(file, line))
        {
            std::cerr << "Unexpected end of file while reading scattering data." << std::endl;
            return false;
        }
        std::istringstream scatIss(line);
        if (!std::getline(scatIss, token, ','))
        {
            std::cerr << "Unexpected end of file while reading scattering data." << std::endl;
            return false;
        }
        if (token != materialName)
        {
            std::cerr << "Material name mismatch between absorption and scattering data: " << materialName << " vs " << token << std::endl;
            return false;
        }
        for (int j = 0; j < numFreqBands; ++j)
        {
            if (!std::getline(scatIss, token, ','))
            {
                std::cerr << "Insufficient scattering data for material " << materialName << "." << std::endl;
                return false;
            }
            scattering[j] = std::stof(token);
        }
    }
    return true;
}

////////////////////////////////////////

bool LoadIndexingFromCsv(const std::string& indexingFilePath, Matrix<int>& outPathIndexing, int& numPaths)
{
    std::ifstream file(indexingFilePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open path indexing csv file: " << indexingFilePath << std::endl;
        return false;
    }

    std::string line;

    // Skip comments and header
    do
    {
        if (!std::getline(file, line))
        {
            std::cerr << "Unexpected end of file while reading header." << std::endl;
            return false;
        }
    } while (!line.empty() && line[0] == '%');

    std::istringstream iss(line);
    int numRows = 0;
    int numCols = 0;
    iss >> numRows >> numCols >> numPaths;

    if (numRows != numCols)
    {
        std::cerr << "Path indexing matrix must be square." << std::endl;
        return false;
    }

    if (numRows <= 0)
    {
        std::cerr << "Invalid number of nodes in path indexing csv file." << std::endl;
        return false;
    }
    outPathIndexing = Matrix<int>::Constant(numRows, numCols, -1);

    int row, col, val;
    for (int k = 0; k < numPaths; ++k) {
        if (!std::getline(file, line))
        {
            std::cerr << "Unexpected end of file while reading entries." << std::endl;
            return false;
        }

        std::istringstream entryIss(line);
        if (!(entryIss >> row >> col >> val))
        {
            std::cerr << "Failed to parse entry line: " << line << std::endl;
            return false;
        }

        if (val < 1 || val > numPaths)
        {
            std::cerr << "Invalid path index value (" << row << ", " << col << ")." << std::endl;
            return false;
        }

        // MatrixMarket is 1-based -> convert to 0-based
        row--;
        col--;
        val--;

        if (row < 0 || row >= numRows || col < 0 || col >= numCols)
        {
            std::cerr << "Invalid index (" << row << ", " << col << ") in mtx file." << std::endl;
            return false;
        }
        outPathIndexing(row, col) = val;
    }
    return true;
}

////////////////////////////////////////

bool LoadModesFromCsv(const std::string& modalDataFilePath, int numFreqBands, int numPaths, Vec<int>& outFrequencyBandIndexing, Vec<>& outT60s, std::vector<Vec<>>& outLeftEigenvectors, std::vector<Vec<>>& outRightEigenvectors)
{
    std::ifstream file(modalDataFilePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open modal data csv file: " << modalDataFilePath << std::endl;
        return false;
    }

    std::string line;
    std::string token;

    std::vector<int> freqBands;
    std::vector<Real> t60s;
    outLeftEigenvectors.clear();
    outRightEigenvectors.clear();

    int fdnIdx = 0;
    while (std::getline(file, line))
    {
        std::istringstream slopIss(line);
        if (!std::getline(slopIss, token, ','))
        {
            std::cerr << "Missing frequency band data for fdn " << fdnIdx << "." << std::endl;
            return false;
        }
        freqBands.push_back(std::stoi(token));

        if (freqBands[fdnIdx] < 0 || freqBands[fdnIdx] >= numFreqBands)
        {
            std::cerr << "Invalid frequency band index for fdn " << fdnIdx << "." << std::endl;
            return false;
        }

        if (!std::getline(slopIss, token, ','))
        {
            std::cerr << "Missing t60 data for fdn " << fdnIdx << "." << std::endl;
            return false;
        }
        t60s.push_back(std::stof(token));

        // Read left eigenvector line
        if (!std::getline(file, line))
        {
            std::cerr << "Unexpected end of file while reading left eigenvector data." << std::endl;
            return false;
        }
        outLeftEigenvectors.emplace_back(numPaths);
        std::istringstream lEigenIss(line);
        for (int i = 0; i < numPaths; ++i)
        {
            if (!std::getline(lEigenIss, token, ','))
            {
                std::cerr << "Insufficient left eigenvector data for fdn " << fdnIdx << "." << std::endl;
                return false;
            }
            outLeftEigenvectors[fdnIdx](i) = std::stof(token);
        }

        // Read left eigenvector line
        if (!std::getline(file, line))
        {
            std::cerr << "Unexpected end of file while reading left eigenvector data." << std::endl;
            return false;
        }
        outRightEigenvectors.emplace_back(numPaths);
        std::istringstream rEigenIss(line);
        for (int i = 0; i < numPaths; ++i)
        {
            if (!std::getline(rEigenIss, token, ','))
            {
                std::cerr << "Insufficient right eigenvector data for fdn " << fdnIdx << "." << std::endl;
                return false;
            }
            outRightEigenvectors[fdnIdx](i) = std::stof(token);
        }
    }

    outFrequencyBandIndexing = Vec<int>(SizeToInt(freqBands.size()));
	std::copy(freqBands.begin(), freqBands.end(), outFrequencyBandIndexing.begin());

	outT60s = Vec<>(SizeToInt(t60s.size()));
	std::copy(t60s.begin(), t60s.end(), outT60s.begin());

    return true;
}

////////////////////////////////////////

bool SendWallsToRAC(const std::string& modartFolder, const Coefficients<>& targetFreqs, Coefficients<>& frequencies)
{
    std::unordered_map<std::string, Coefficients<>> absorption;
    std::unordered_map<std::string, Coefficients<>> scattering;
    std::string mtlFilePath = modartFolder + "/materials.csv";

    if (!LoadMaterialsFromCsv(mtlFilePath, frequencies, absorption, scattering))
    {
        std::cerr << "Failed to parse MTL csv file." << std::endl;
        return false;
    }

    std::vector<Vec3> vData;
    std::vector<Face> faces;
    std::string objFilePath = modartFolder + "/mesh.obj";

    if (!ParseObjFile(objFilePath, vData, faces))
    {
        std::cerr << "Failed to parse OBJ file." << std::endl;
        return false;
    }

    for (const auto& face : faces) {
        int patchIndex = face.patchIndex;
        std::string matName = face.materialName;

        auto it = absorption.find(matName);
        if (it == absorption.end())
        {
            std::cerr << "Material " << matName << " not found in materials data." << std::endl;
            continue;
        }

        Coefficients<> absResized = ResizeCoeffs(targetFreqs, frequencies, it->second);
        UpdateMaterial(patchIndex, absResized);

        // Convert x, y, z to -x, z, y (RAC uses Unity coordinate system, left hand rule and y is up)
        Vertices vertices = {
            -vData[face.v[0]],
            vData[face.v[2]],
            vData[face.v[1]]
        };
        InitWall(vertices, patchIndex);
    }
    UpdatePlanesAndEdges();
    return true;
}

////////////////////////////////////////

bool LoadMoDARTScene(const std::string& modartPath, const Coefficients<>& targetFreqs, const LateReverbData& data)
{
    Coefficients<> frequencies(1);
    if (!SendWallsToRAC(modartPath, targetFreqs, frequencies))
    {
        std::cerr << "Failed to load walls from " << modartPath << std::endl;
        return false;
    }
    int numFreqBands = frequencies.Length();

    int numPaths;
    Matrix<int> pathIndexing;
    if (!LoadIndexingFromCsv(modartPath + "/path_indexing.mtx", pathIndexing, numPaths))
    {
        std::cerr << "Failed to load indexing data from " << modartPath << std::endl;
        return false;
    }
    int numNodes = pathIndexing.Rows();

    Vec<int> frequencyBandIndexing;
    Vec<> t60s;
    std::vector<Vec<>> leftEigenvectors;
    std::vector<Vec<>> rightEigenvectors;
    if (!LoadModesFromCsv(modartPath + "/MoD-ART.csv", frequencies.Length(), numPaths, frequencyBandIndexing, t60s, leftEigenvectors, rightEigenvectors))
    {
        std::cerr << "Failed to load modal data from " << modartPath << std::endl;
        return false;
    }
    int numFDNs = frequencyBandIndexing.Length();

    // Internal parameters `bandIdxs, T60s, leftVecs, rightVecs, numFDNs` match what was read from the files.
    // They need to be truncated/repeated to match the current frequency bands of RAC.
    std::vector<int> resizedFreqBandIndexing;
    std::vector<Real> resizedT60s;
    std::vector<Vec<>> resizedLeftEigenvectors;
    std::vector<Vec<>> resizedRightEigenvectors;

    // Fill out all resized variables.
    int oldBandIdx, newBandIdx;
    int numAssigned = 0;
    for (int localIdx = 0; localIdx < numFDNs; ++localIdx)
    {
        // Find the best match for this slope's frequency band among the requested bands.
        oldBandIdx = frequencyBandIndexing(localIdx);
        newBandIdx = BestBandMatch(frequencies[oldBandIdx], targetFreqs);

        // Only record this slope if its frequency band matches one of the requested bands.
        // If the user removed any of the octave bands through the GUI, some slopes will not find a match and will be ignored.
        if (std::abs(frequencies[oldBandIdx] - targetFreqs[newBandIdx]) < EPS)
        {
            resizedFreqBandIndexing.push_back(newBandIdx);
            resizedT60s.push_back(t60s(localIdx));
            resizedLeftEigenvectors.push_back(leftEigenvectors[localIdx]);
            resizedRightEigenvectors.push_back(rightEigenvectors[localIdx]);
        }
    }
    // TODO: Currently assuming RAC does not include frequency bands not covered by MoD-ART frequencies.

    Vec<int> newFreqBandIndexing(SizeToInt(resizedFreqBandIndexing.size()));
    std::copy(resizedFreqBandIndexing.begin(), resizedFreqBandIndexing.end(), newFreqBandIndexing.begin());

    Vec<> newT60s(SizeToInt(resizedT60s.size()));
    std::copy(resizedT60s.begin(), resizedT60s.end(), newT60s.begin());

    Real delay = 0.1;
    Real minT60 = 0.2;
    MoDARTData modartData(true, data.numRays, data.feedbackMatrix, delay, minT60, pathIndexing, newFreqBandIndexing, newT60s, resizedLeftEigenvectors, resizedRightEigenvectors);
    return InitMoDART(modartData);
}