# Unity Interface

The Unity interface provides a set of modular c# scripts that can be used to integrate RoomAcoustiC++ into Unity projects
The audio processing is built on top of the default Unity audio system.
The interface is available on the GitHub repository [here](https://github.com/jmannall/RoomAcoustiCpp-Unity).

## Tutorials

The following section provides short tutorials covering the different aspects of the RoomAcoustiC++ Unity interface.
They are designed to help you get started with using the interface and understand its features.

COMING SOON: Video tutorial series.

### Getting started (Recommended for first time users)

- [Installation](installation.md): How to add RoomAcoustiCpp-Unity to your Unity project.
- [Beginner introduction](introduction.md): An overview of the RoomAcoustiC++ Unity interface and how to create a simple scene.

It is possible to use RoomAcoustiCpp-Unity by simply adding the premade C# scripts to the relevant GameObjects in your scene.
However, if more interactivity is required, you will need to write custom scripts to control the behavior of the audio sources and listeners. For those unfamiliar with Unity the following tutorials are recommended as a starting point:

- [The Unity Tutorial For Complete Beginners](https://www.youtube.com/watch?v=XtQMytORBmM) – Game Maker’s Toolkit
- [Introduction to Game Development](https://www.youtube.com/playlist?list=PLFt_AvWsXl0fnA91TcmkRyhhixX9CO3Lw) – Sebastian Lague

### Specific tutorials
- [Creating an audio listener](racaudiolistener.md): How to create the audio listener.
- [Creating an audio source](racaudiosource.md): How to create and configure audio sources.
- [Creating scene geometry](racmesh.md): How to create room geometry and configure the room acoustic properties.
- [Adding material properties](racmaterial.md): How to create a new material and assign it to the room geometry.
- [Configuring the image edge model](imageedge.md): How to configure the image edge model for the room.
- [Changing the RoomAcoustiC++ plugin](plugin.md): How to change the active RoomAcoustiC++ plugin.
- [Receive debug messages and visualise the acoustic model](debug.md): How to use the `DebugCpp` script to aid debugging.
- [Custom HRTF](hrtf.md): How to create and use a custom HRTF (Head-Related Transfer Function).
