# RoomAcoustiC++ Documentation

Welcome to the official **RoomAcoustiC++ (RAC)** documentation site.
The original version of RAC can be found [here](https://github.com/jmannall/RoomAcoustiCpp), and this software is an extension of it.

RAC is a C++ library for real-time room acoustic modelling designed for psychoacoustic research and immersive audio applications.
It uses the image-edge model (IEM) for early reflections and low-order diffraction, and uses the modal decomposition of acoustic radiance transfer (MoD-ART) for convolution-free late reverberation.
The library provides a simple API interface for controlling source and listener positions, room geometry, and acoustic parameters.

## Quick start

* If you are building a C++ audio program you can integrate the RAC library into your own application that handles the appropriate function calls to the API.
* If you are a researcher or developer looking to use RAC you can use the (soon to be released) Unity or Unreal interface for efficient and simple control of the acoustic model.

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

## Copyright and License

The software is currently under a General Public License (GPL) due to the inclusion of the 3D Tune-In (3DTI) toolbox, which is used for spatialization.
We are planning to replace 3DTI with an equivalent toolbox which would allow our code to be either MIT or LGPL.
