# RoomAcoustiC++ Documentation

Welcome to the official **RoomAcoustiC++ (RAC)** documentation site.
The original version of RAC can be found [here](https://github.com/jmannall/RoomAcoustiCpp), and this software is an extension of it.

RAC is a C++ library for real-time room acoustic modelling designed for psychoacoustic research and immersive audio applications.
It uses the image-edge model (IEM) for early reflections and low-order diffraction, and feedback delay networks (FDN) for convolution-free late reverberation.
Two late reverberation models are implemented:
* The modal decomposition of acoustic radiance transfer (MoD-ART) that is suitable for indoor, outdoor and complex multi-room environments
* A single FDN approach (similar to RAZR) suitable for simple single room environments

The library provides a simple API interface for controlling source and listener positions, room geometry, and acoustic parameters.

## Quick start

* [RoomAcoustiC++](rac/index.md): If you are looking to learn more about the RoomAcoustiC++ library and the underlying acoustic models.
* [C++ Interface](api/index.md): If you are building a C++ audio program you can integrate the RAC library into your own application that handles the appropriate function calls to the API.
* [Unity Interface](unity/index.md): If you are a researcher or developer looking to use RAC you can use the Unity or (soon to be released) Unreal interface for efficient and simple control of the acoustic model (suitable for users with no or minimal coding experience).

## Credits

This software was developed by

* [Joshua Mannall](https://github.com/jmannall) ([Institute of Sound Recording, University of Surrey](https://iosr.surrey.ac.uk/)). Contact: j.mannall@surrey.ac.uk
* [Matteo Scerbo](https://github.com/Matteo-Scerbo) ([Institute of Sound Recording, University of Surrey](https://iosr.surrey.ac.uk/)). Contact: m.scerbo@surrey.ac.uk
* [Daniel Sass](https://github.com/twindan). Contact: dan@sass.org
* [Enzo De Sena](https://github.com/enzodesena) ([Institute of Sound Recording, University of Surrey](https://iosr.surrey.ac.uk/)). Contact: e.desena@surrey.ac.uk
* [Randall Ali](https://github.com/randyaliased) ([Institute of Sound Recording, University of Surrey](https://iosr.surrey.ac.uk/)). Contact: r.ali@surrey.ac.uk

Joshua is the original developer of RAC, and is responsible for the IEM components as well as most of the digital signal processing.
Matteo is responsible for the MoD-ART components.

Daniel is responsible for most of the optimization work.

Enzo and Randy assisted in the development of the MoD-ART model.

## License

RoomAcoustiC++ is distributed under the GPL v3, a popular open-source license with strong copyleft conditions license.

If you license RoomAcoustiCpp under GPL v3, there is no license fee or signed license agreement: you just need to comply with the GPL v3 terms and conditions. See the license files on the GitHub repository for more information.

## Acknowledgements

The project utilises the [3D-TuneIn Toolkit](https://github.com/3DTune-In/3dti_AudioToolkit) (3DTI) for spatialisation.
The forked 3dti_AudioToolkit repository (included as a submodule) includes all the required files for use with RAC.

The lock-free queue [concurrentqueue](https://github.com/cameron314/concurrentqueue) is used for multithreaded audio processing.
