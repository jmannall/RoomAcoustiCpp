/**
* @brief Scene examples for profiling. Includes a main entry point for building an executable.
*/

#include <iostream>

#include "Spatialiser/Interface.h"

#include "MoDARTLoader.h"

using namespace RAC::Spatialiser;
using namespace RAC::Common;
using namespace RAC::DSP;

std::vector<size_t> CreateShoeboxRoom(Vec3 pos, size_t materialId)
{
    double volume = pos.x * pos.y * pos.z;
    std::vector<size_t> wallIDs(12);
    wallIDs[0] = InitWall({ Vec3(0.0, pos.y, 0.0),
            Vec3(pos.x, pos.y, 0.0),
            Vec3(pos.x, pos.y, pos.z) }, materialId);
    wallIDs[1] = InitWall({ Vec3(0.0, pos.y, 0.0),
            Vec3(pos.x, pos.y, pos.z),
            Vec3(0.0, pos.y, pos.z) }, materialId);
    wallIDs[2] = InitWall({ Vec3(pos.x, 0.0, 0.0),
            Vec3(0.0, 0.0, 0.0),
            Vec3(0.0, 0.0, pos.z) }, materialId);
    wallIDs[3] = InitWall({ Vec3(pos.x, 0.0, 0.0),
            Vec3(0.0, 0.0, pos.z),
            Vec3(pos.x, 0.0, pos.z) }, materialId);
    wallIDs[4] = InitWall({ Vec3(pos.x, 0.0, pos.z),
            Vec3(pos.x, pos.y, pos.z),
            Vec3(pos.x, pos.y, 0.0) }, materialId);
    wallIDs[5] = InitWall({ Vec3(pos.x, 0.0, pos.z),
            Vec3(pos.x, pos.y, 0.0),
            Vec3(pos.x, 0.0, 0.0) }, materialId);
    wallIDs[6] = InitWall({ Vec3(0.0, 0.0, 0.0),
            Vec3(0.0, pos.y, 0.0),
            Vec3(0.0, pos.y, pos.z) }, materialId);
    wallIDs[7] = InitWall({ Vec3(0.0, 0.0, 0.0),
            Vec3(0.0, pos.y, pos.z),
            Vec3(0.0, 0.0, pos.z) }, materialId);
    wallIDs[8] = InitWall({ Vec3(0.0, 0.0, 0.0),
            Vec3(pos.x, 0.0, 0.0),
            Vec3(pos.x, pos.y, 0.0) }, materialId);
    wallIDs[9] = InitWall({ Vec3(0.0, 0.0, 0.0),
            Vec3(pos.x, pos.y, 0.0),
            Vec3(0.0, pos.y, 0.0) }, materialId);
    wallIDs[10] = InitWall({ Vec3(0.0, pos.y, pos.z),
            Vec3(pos.x, pos.y, pos.z),
            Vec3(pos.x, 0.0, pos.z) }, materialId);
    wallIDs[11] = InitWall({ Vec3(0.0, pos.y, pos.z),
            Vec3(pos.x, 0.0, pos.z),
            Vec3(0.0, 0.0, pos.z) }, materialId);
    UpdatePlanesAndEdges();
	return wallIDs;
}

void ProfileShoebox()
{
    std::cout << "Init RAC..." << std::endl;
    int fs{ 48000 };							// Sample rate
    int numFrames{ 512 };						// Number of frames per audio callback
    int numReverbSources{ 12 };					// Number of output channels for late reverberation
    int fdnSize{ 12 };							// Size of the FDN (number of delay lines)
    Real lerpFactor = 2.0; 				        // Interpolation factor
    Real Q{ 0.98 };								// Q factor for the GraphicEQ
    Coefficients<> frequencyBands({ 125.0, 250.0, 500.0, 1e3, 2e3, 4e3, 8e3 });				// Frequency band center frequencies

    DSPData configData = DSPData(fs, numFrames, numReverbSources, fdnSize, lerpFactor, Q, frequencyBands);
    Init(configData);

    int hrtfSamplingStep = 5;
    std::vector<std::string> hrtfFiles = { "HRTF/Kemar_DTF_ITD_48000_3dti-hrtf.3dti-hrtf", "HRTF/NearFieldCompensation_ILD_48000.3dti-ild", "HRTF/HRTF_ILD_48000.3dti-ild" };
    bool success = LoadSpatialisationFiles(hrtfSamplingStep, hrtfFiles);
    if (!success)
    {
        std::cout << "Error loading spatialisation files!" << std::endl;
        UpdateSpatialisationMode(SpatialisationMode::none);
    }
    else
        UpdateSpatialisationMode(SpatialisationMode::quality);

    DirectSound dir = DirectSound::doCheck;
    int reflOrder = 2;
    int shadowOrder = 2;
    int specularOrder = 1;
    Real minEdgeLength = 0.0;
    Real maxPathLength = 1e4; // No limit on path length
    EarlyReverbData earlyReverbData(dir, reflOrder, shadowOrder, specularOrder, minEdgeLength, maxPathLength);

    DiffractionModel diffractionModel = DiffractionModel::nnSmall;

    InitEarlyReverb(true, earlyReverbData, diffractionModel);

    // Create shoebox
    Vec3 pos(7.0, 3.0, 4.0);
    Absorption<> absorption({ 0.03, 0.03, 0.04, 0.06, 0.09, 0.1, 0.12 });
	size_t materialId = InitMaterial(absorption);
    auto wallIds = CreateShoeboxRoom(pos, materialId);

    int numRays = 1e3;
    FDNMatrix matrix = FDNMatrix::randomOrthogonal;

    Real volume = pos.x * pos.y * pos.z;
    Coefficients<> t60({ 0.8, 0.7, 0.65, 0.63, 0.6, 0.55, 0.48 });
    ReverbFormula formula = ReverbFormula::Custom;
    Vec<> dimensions({ pos.x, pos.y, pos.z });
    RoomData roomData(volume, t60, formula, dimensions);

    LateReverbData lateReverbData(true, numRays, matrix);

    InitSingleFDN(roomData, lateReverbData);

    Vec3 listenerPos(0.0, 2.0, 0.0);
    Vec4 listenerOri(1.0, 0.0, 0.0, 0.0);
    UpdateListener(listenerPos, listenerOri);

    Vec3 sourcePos(1.0, 2.0, 3.0);
    Vec4 sourceOri(1.0, 0.0, 0.0, 0.0);

    // Stereo output buffer
    int numBuffers = 10;
    Buffer<> output(2 * numBuffers * configData.numFrames);

    std::cout << "Submit audio..." << std::endl;
    RecordImpulseResponse(sourcePos, sourceOri, output);

    std::cout << "Exit RAC..." << std::endl;
    for (size_t wallID : wallIds)
        RemoveWall(wallID);
	RemoveMaterial(materialId);
    Exit();
}

void ProfileMoDART()
{
    std::cout << "Init RAC..." << std::endl;
    int fs{ 48000 };							// Sample rate
    int numFrames{ 512 };						// Number of frames per audio callback
    int numReverbSources{ 12 };					// Number of output channels for late reverberation
    int fdnSize{ 12 };							// Size of the FDN (number of delay lines)
    Real lerpFactor = 2.0; 				        // Interpolation factor
    Real Q{ 0.98 };								// Q factor for the GraphicEQ
    Coefficients<> frequencyBands({ 125.0, 250.0, 500.0, 1e3, 2e3, 4e3, 8e3 });				// Frequency band center frequencies

    DSPData configData = DSPData(fs, numFrames, numReverbSources, fdnSize, lerpFactor, Q, frequencyBands);
    Init(configData);

    int hrtfSamplingStep = 5;
    std::vector<std::string> hrtfFiles = { "HRTF/Kemar_DTF_ITD_48000_3dti-hrtf.3dti-hrtf", "HRTF/NearFieldCompensation_ILD_48000.3dti-ild", "HRTF/HRTF_ILD_48000.3dti-ild" };
    bool success = LoadSpatialisationFiles(hrtfSamplingStep, hrtfFiles);
    if (!success)
    {
        std::cout << "Error loading spatialisation files!" << std::endl;
        UpdateSpatialisationMode(SpatialisationMode::none);
    }
    else
        UpdateSpatialisationMode(SpatialisationMode::quality);

    DirectSound dir = DirectSound::doCheck;
    int reflOrder = 2;
    int shadowOrder = 2;
    int specularOrder = 1;
    Real minEdgeLength = 0.0;
    Real maxPathLength = 1e4; // No limit on path length
    EarlyReverbData earlyReverbData(dir, reflOrder, shadowOrder, specularOrder, minEdgeLength, maxPathLength);

    DiffractionModel diffractionModel = DiffractionModel::nnSmall;

    InitEarlyReverb(true, earlyReverbData, diffractionModel);

    // Load MoDART scene
    int numRays = 1e3;
    FDNMatrix matrix = FDNMatrix::randomOrthogonal;
    LateReverbData lateReverbData(true, numRays, matrix);

    std::string modartPath = "MoDART";
    if (!LoadMoDARTScene(modartPath, frequencyBands, lateReverbData))
    {
		std::cout << "Error loading MoDART scene!" << std::endl;
        Exit();
        return;
    }

    Vec3 listenerPos(2.0, 1.0, 6.8);
    Vec4 listenerOri(1.0, 0.0, 0.0, 0.0);
    UpdateListener(listenerPos, listenerOri);

    Vec3 sourcePos(2.0, 1.5, 2.0);
    Vec4 sourceOri(1.0, 0.0, 0.0, 0.0);

    // Stereo output buffer
    int numBuffers = 10;
    Buffer<> output(2 * numBuffers * configData.numFrames);

    std::cout << "Submit audio..." << std::endl;
    RecordImpulseResponse(sourcePos, sourceOri, output);

    std::cout << "Exit RAC..." << std::endl;
    /*for (size_t wallID : wallIds)
        RemoveWall(wallID);
    RemoveMaterial(materialId);*/
    Exit();
}

int main() {
    std::cout << "Start program!" << std::endl;
    std::cout << "Enter a profile key or press q to exit." << std::endl;

    char key;
    while (true)
    {
        std::cin >> key;

        switch (key)
        {
        default:
            std::cout << "No profile exists for this key. Enter a valid key or press q to exit." << std::endl;
            continue;
        case '1':
            ProfileShoebox();
            break;
        case '2':
            ProfileMoDART();
            break;
        case 'q':
            std::cout << "Exiting program!" << std::endl;
            return 0;
        }
        std::cout << "Profile run complete!" << std::endl;
    }
}