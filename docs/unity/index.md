# Unity Interface

The Unity interface provides a set of modular c# scripts that can be used to integrate RoomAcoustiC++ into Unity projects.
The audio processing is built on top of the default Unity audio system.

## RACManager
The `RACManager` script is the main entry point for using RoomAcoustiC++ in Unity.
It handles the initialisation of the spatialiser and manages the acoustic model.

## RACListener
The `RACListener` script must be attached to the main camera or any GameObject that represents the listener in the scene.

## RACSource
The `RACSource` script should be attached to any GameObject that represents a sound source in the scene.
The transform of the GameObject will be used to determine the position and orientation of the sound source.
It overrides the default Unity AudioSource functions and the directivity and gain of the sound can be modified in the inspector

## RACMesh
The `RACMesh` script should be attached to a GameObject as a parent to the mesh representing the scene geometry.
It will automatically create and update walls based on the mesh geometry and apply the absorption properties defined in the `RACMaterial` script.

## RACMaterial
The `RACMaterial` script is used to store `RACMaterialEntry` objects.

## RACMaterialEntry
`RACMaterialEntry` is a ScriptableObject that stores the absorption properties of a material.
A number of preset materials are provided.
The material entry is paired with a Unity material.

## RACDebug
The `RACDebug` script is used to visualise the RoomAcoustiC++ spatialisation system in Unity.
If the RAC_Debug plugin is used and gizmos are enabled, it will draw the current edges, image source paths and reverb source positions in the scene view.