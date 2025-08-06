# RoomAcoustiC++: Real-time Acoustics Library

RoomAcoustiC++ (RAC) is a C++ library for real-time room acoustic modelling designed for psychoacoustic research and immersive audio applications.
RAC was developed as part of a PhD at the University of Surrey and is distributed open-source with the aim of encouraging and increasing accessibility to real-time acoustics across technical and non-technical research fields.

The official documentation of RoomAcoustiC++, including the Unity Engine interface is available here [https://roomacousticpp.readthedocs.io/en/latest/](https://roomacousticpp.readthedocs.io/en/latest/).

## Citable references
First publication describing the architecture of RAC, its main features, and analysis of accuracy and real-time performance:
* Mannall J., Savioja L., Neidhardt A., Mason R. and De Sena E. "RoomAcoustiC++: An open-source room acoustic model for real-time audio simulations,” in Proc. AES Int. Conf. on Headphone Tech., Espoo, Finland, 2025

## Additional tools
In addition to this main repository, there are other repositories with associated tools:
* RoomAcoustiCpp-Unity: Modular C# scripts and resources for straight foward integration of RAC with the Unity Engine. Code available at: [https://github.com/jmannall/RoomAcoustiCpp-Unity](https://github.com/jmannall/RoomAcoustiCpp-Unity)

## Credits
The software is being developed by
* [Joshua Mannall](https://github.com/jmannall) ([Institute of Sound Recording, University of Surrey](https://iosr.surrey.ac.uk/)). Contact: j.mannall@surrey.ac.uk

## Aknowledgements
The project utilises the [3D-TuneIn Toolkit](https://github.com/3DTune-In/3dti_AudioToolkit) (3DTI) for binaural processing.
The forked 3dti_AudioToolkit repository (included as a submodule) includes all the required files for use with RAC.
Some small changes have been made for the purposes of compatibility between the source files.

The lock-free queue [concurrentqueue](https://github.com/cameron314/concurrentqueue) is used for multithreaded audio processing.

## License
RoomAcoustiCpp is distributed under the GPL v3, a popular open-source license with strong copyleft conditions license.

If you license RoomAcoustiCpp under GPL v3, there is no license fee or signed license agreement: you just need to comply with the GPL v3 terms and conditions. See ROOMACOUSTICPP_LICENSE and LICENSE for further information.
