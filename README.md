# RoomAcoustiC++: Real-time Acoustics Library
## Description
RoomAcoustiC++ (RAC) is a C++ library for real-time room acoustic modelling.
At a high level it consists of a combined image edge ([IE](https://doi.org/10.1051/aacus/2021010)) and feedback delay network (FDN) model that can be applied to an arbitrary triangulated mesh.
The repo includes visual studio projects for building a DLL (windows) or SO (linux/android) library that can be interfaced from other applications.
There are plans to add a template for use of the library within the Unity game engine.

## Capabilities
* 6 degrees of freedom for the source and receiver
* Dynamic geometry
* Binaural spatialisation including custom HRTFs
* Non-shoebox rooms
* Geometry dependent early reflections
* Diffraction modelling
* FDN late reverberation

## Acoustic model
The IE model configuration is user controlled.
This includes toggling direct sound, reflection, diffraction, diffracted reflection paths, late reverberation and setting the maximum reflection/diffraction order of the model.
Multiple diffraction models are implemented including: Biot-Tolstoy-Medwin ([BTM](https://doi.org/10.1121/1.428071)), Uniform Theory of Diffraction ([UTD](https://doi.org/10.1109/PROC.1974.9651)), Neural network model ([NN-IIR](https://doi.org/10.17743/jaes.2022.0107)) and Universal Diffraction FIlter Approximation ([UDFA](https://doi.org/10.1109/TASLP.2023.3264737)).
The FDN uses either a random orthogonal or ... feedback matrix and uses absorption filters to control the reverberation time (RT60).
This can be user defined or set using Sabine or Eyring's formulae.

## Getting started
The central control of the software is in the [context.h](DiffractionPlugin/include/Spatialiser/Context.h) file.
The [main.cpp](DiffractionPlugin/source/Spatialiser/Main.cpp) file includes all the functions exposed to the DLL and SO plugins.
The full acoustic model can be controlled through these functions.

## Aknowledgements
The project utilises the 3D Tune-in toolbox for binaural processing.

## License
