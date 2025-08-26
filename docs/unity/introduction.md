# Beginner introduction

In Unity, objects in a scene are called GameObjects.
The behaviour of GameObjects can be modified by attaching Components and C# scripts to them. For those unfamiliar with Unity the following tutorials are recommended as a starting point:

- [The Unity Tutorial For Complete Beginners](https://www.youtube.com/watch?v=XtQMytORBmM) – Game Maker’s Toolkit
- [Introduction to Game Development](https://www.youtube.com/playlist?list=PLFt_AvWsXl0fnA91TcmkRyhhixX9CO3Lw) – Sebastian Lague

## Overview
RoomAcoustiCpp-Unity provides a number of premade C# scripts that represent the different components of the RAC spatialisation system, making it easy to integrate spatial audio into your Unity projects.

### RACManager
The `RACManager` script is the main entry point for using RAC in Unity.
It handles the initialisation of the spatialiser and manages the acoustic model.

### RACListener
The `RACListener` script must be attached to the main camera or any GameObject that represents the listener in the scene.

### RACSource
The `RACSource` script should be attached to any GameObject that represents a sound source in the scene.
The transform of the GameObject will be used to determine the position and orientation of the sound source.
It overrides the default Unity AudioSource behaviour and the directivity and gain of the sound can be modified in the inspector

### RACMesh
The `RACMesh` script should be attached to a GameObject as a parent to the mesh representing the scene geometry.
It will automatically create and update walls based on the mesh geometry and apply the absorption properties defined in the `RACMaterial` script.

### RACMaterial
The `RACMaterial` script is used to store `RACMaterialEntry` objects.

### RACMaterialEntry
`RACMaterialEntry` is a ScriptableObject that stores the absorption properties of a material.
A number of preset materials are provided.
The material entry is paired with a Unity material.

### RACDebug
The `RACDebug` script is used to visualise the RAC spatialisation system in Unity.
If the RAC_Debug plugin is used and gizmos are enabled, it will draw the current edges, image source paths and reverb source positions in the scene view.

## Create your first scene

The following steps outline how to create a simple scene in Unity and integrate RAC.

1. Create an empty GameObject and attach the RACManager script.
    1. Change the Image Edge Configuration to:
        - Direct: check
        - Reflection: 1
        - Shadow Diffraction: 1
        - Specular Diffraction: 0
        - Late Reverb: true
    2. Set T60 to custom and set the desired reverberation time in seconds for each frequency band (from low to high).
    3. Set the spatialisation mode to `performance`.
2. Add the RACListener script to the main camera or GameObject that represents the listener.
3. Add the RACSource script to a GameObject that represents a sound source in the scene.
    1. Add an audio clip asset to the RACSource GameObject.
    2. Set the playOnAwake property to true.
    3. Set the loop property to true.
4. Create a parent GameObject for your scene geometry and attach the RACMesh script.
5. Add your scene geometry to the scene hierarchy as children of the RACMesh GameObject.
6. Press play! Moving the source and listener around in the scene view will update the spatialisation in real-time.

Explore the rest of the tutorial series for more information of defining absorption parameters, modifying the acoustic model, creating a custom HRTF and more.