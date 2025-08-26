# Overview

The **RoomAcoustiC++ (RAC)** library is a C++ library designed to support researchers and developers in the field of immersive audio and room acoustics.
It was developed for the purpose of perceptual experiments on the plausiblity of acoustic rendering methods for VR and AR scenarios.
Now released open-source, the hope is to enable further research on the perception of room acoustics and provide an accessible and transparent tool for researchers across both technical and non-technical disciplines.

This section provides an overview of the acoustic models implemented within RAC.
It is designed as a high level summary that is relevant to both the [C++](../api/index.md) and [Unity](../unity/index.md) interfaces.

## Features

* Real-time 6 degrees of freedom for the source and listener
* Binaural spatialisation including custom HRTFs (using the 3D Tune-In toolbox)
* Geometry dependent early reflections
* Diffraction modelling
* FDN late reverberation
* Dynamic geometry

## Citable References

If you make use of RAC in your own research or other projects, please consider citing the following publication:

* Mannall J, Savioja L, Neidhardt A, Mason R, and De Sena E. "RoomAcoustiC++: An open-source room acoustic model for real-time audio simulations,” in Proc. AES Int. Conf. Headphone Tech., Espoo, Finland. 1-13, 2025.

<!-- 
## Description

For users simply looking to use the library, a straightforward API is provided.
This exposes functions for creating and manipulating audio sources, listeners, and the room environment.

For developers interested in extending the library, see the [C++ API Documentation](https://roomacousticpp.readthedocs.io/en/latest/common/definitions.html). -->