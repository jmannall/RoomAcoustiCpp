# RoomAcoustiC++ Documentation

Welcome to the official **RoomAcoustiC++ (RAC)** documentation site.
RAC is a C++ library for real-time room acoustic modelling designed for psychoacoustic research and immersive audio applications.
The library provides an intuitive C++ interface for controlling source and listener positions, room geometry and acoustic parameters.

This site covers the following repositories with associated tools:

* [RoomAcoustiCpp](https://github.com/jmannall/RoomAcoustiCpp): C++ source code.
* [RoomAcoustiCpp-Unity](https://github.com/jmannall/RoomAcoustiCpp-Unity): Modular C# scripts and resources for straight foward integration of RAC with the Unity Engine streamlining integration with modern VR and AR tools.

## Quick start

* [C++ Interface](api/index.md): If you are building a C++ audio program you can integrate the RAC library into your own application that handles the appropriate function calls to the API.
* [Unity Interface](unity/index.md): If you are a researcher or developer looking to use RAC you can use the provided Unity interface for efficient and simple control of the acoustic model (suitable for users with no or minimal coding experience).

## Credits

This software is being developed by:

* [Joshua Mannall](https://github.com/jmannall) ([Institute of Sound Recording, University of Surrey](https://iosr.surrey.ac.uk/)). Contact: j.mannall@surrey.ac.uk

## License

RoomAcoustiCpp is distributed under the GPL v3, a popular open-source license with strong copyleft conditions license.

If you license RoomAcoustiCpp under GPL v3, there is no license fee or signed license agreement: you just need to comply with the GPL v3 terms and conditions. See the license files on the GitHub repository for more information.

## Acknowledgements

The project utilises the [3D-TuneIn Toolkit](https://github.com/3DTune-In/3dti_AudioToolkit) (3DTI) for binaural processing.
The forked 3dti_AudioToolkit repository (included as a submodule) includes all the required files for use with RAC.
Some small changes have been made for the purposes of compatibility between the source files.

The lock-free queue [concurrentqueue](https://github.com/cameron314/concurrentqueue) is used for multithreaded audio processing.
