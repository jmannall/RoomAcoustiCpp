# Creating room geometry

To add geometry to RoomAcoustiC++, create an empty GameObject and add the `RACMesh` script.
Assign any 3D objects (e.g. quads, cubes) as children of the GameObject with the `RACMesh` component.
Quads are recommended over planes due to the reduced number of triangles in the default Unity mesh.

## Modify acoustic properties

A number of properties can be modified in the `RACMesh` component to control the acoustic behaviour of the room.

- **Room Dimensions**: The primary dimensions of the room in meters (for a shoebox room this would be the width, height, depth). This controls how quickly the late reverberation echoes build up. Larger dimensions result in longer delay lines with the FDN and a sparser build up of late reverberation.
- **Volume**: The volume of the room in cubic meters. This is used to calculate the late reverberation time (T60) if a Sabine or Eyring's formula is selected in the `RACManager`. A larger volume results in a longer reverberation time.
- **Absorption Skew**: This scales the absorption coefficients of the mesh surfaces. This affects both early reflections and late reverberation (if using Sabine or Eyring's formula).

## Disable Mesh Renderers

It can be useful to use a simplified mesh with a low number of triangles for acoustic rendering.
This reduces the computational load for the image edge model and ensures any latency due to updating the acoustic model is minimized.
A second mesh with a higher triangle count can be used for visual representation without affecting the acoustic calculations.
To simplify this process `RACMesh` provides an option to disable the mesh renderers of all child objects at runtime.

- **Disable Mesh Renderers**: If enabled, this will disable the mesh renderers of all child objects at runtime.