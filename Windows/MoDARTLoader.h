/*
* @brief Utility functions to load MoDART preprocessed data from files.
*
*/

#include "Spatialiser/Interface.h"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>

#ifndef RoomAcoustiCpp_MoDARTLoader_h
#define RoomAcoustiCpp_MoDARTLoader_h

using namespace RAC::Spatialiser;
using namespace RAC::Common;
using namespace RAC::DSP;

/**
* @brief Helper struct for a face(triangle) in the .obj file
*/
struct Face
{
    int v[3];
    std::string materialName;
    int patchIndex;
};

bool ParseObjFile(const std::string& objFilePath, std::vector<Vec3>& outVertices, std::vector<Face>& outFaces);

int BestBandMatch(float targetBand, Coefficients<> referenceBands);

Absorption ResizeCoeffs(Coefficients<> targetFreqs, Coefficients<> inputFreqs, Absorption inputCoeffs);

bool LoadMaterialsFromCsv(const std::string& mtlFilePath, Coefficients<>& outFrequencies, std::unordered_map<std::string, Absorption>& outAbsorption, std::unordered_map<std::string, Coefficients<>>& outScattering);

bool LoadIndexingFromCsv(const std::string& indexingFilePath, Matrix<int>& outPathIndexing, int& numPaths);

bool LoadModesFromCsv(const std::string& modalDataFilePath, int numFreqBands, int numPaths, Vec<int>& outFrequencyBandIndexing, Vec<>& outT60s, std::vector<Vec<>>& outLeftEigenvectors, std::vector<Vec<>>& outRightEigenvectors);

bool SendWallsToRAC(const std::string& modartFolder, const Coefficients<>& targetFreqs, Coefficients<>& frequencies);

bool LoadMoDARTScene(const std::string& modartPath, const Coefficients<>& targetFreqs, const LateReverbData& data);

#endif // RoomAcoustiCpp_MoDARTLoader_h
