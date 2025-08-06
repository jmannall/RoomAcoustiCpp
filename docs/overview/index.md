# RoomAcoustiC++: Overview

The **RoomAcoustiC++ (RAC)** library is a C++ library designed to support researchers and developers in the field of immersive audio and room acoustics.
This section provides an overview of the library's features.

## Features

* Real-time 6 degrees of freedom for the source and listener
* Binaural spatialisation including custom HRTFs (using the 3D Tune-In toolbox)
* Geometry dependent early reflections
* Diffraction modelling
* FDN late reverberation
* Dynamic geometry

## Installation

To use the C++ library, you need to add the RoomAcoustiC++ source and header files to your project and add `#!cpp #include "Spatialiser/Interface.h"`.
You can clone the repository using the following command:

```bash
git clone --recurse-submodules https://github.com/jmannall/RoomAcoustiCpp.git
```

<!-- 
## Description

For users simply looking to use the library, a straightforward API is provided.
This exposes functions for creating and manipulating audio sources, listeners, and the room environment.

For developers interested in extending the library, see the [C++ API Documentation](https://roomacousticpp.readthedocs.io/en/latest/common/definitions.html). -->