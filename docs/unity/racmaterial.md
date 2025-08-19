# Adding material properties

Once you have created your room geometry and assigned the `RACMesh` script as described in [Creating room geometry](racmesh.md), you can assign materials to the geometry.
The `RACMaterial` script stores a list of `RACMaterialEntry` ScriptableObjects and handles the assignment of absorption parameters to the geometry.
It should be attached to the same GameObject as the `RACMesh` script.

## Create a material

Materials are stored as ScriptableObjects in the project files.
This means they can be used between multiple scenes.
They store the frequency dependent absorption properties for a given material.
For the experienced and less experienced user they provide a number of preset materials to choose from that are taken from databases of real-world measurements.
Assigning materials to geometry is achieved by linking a Unity `Material` to a `RACMaterialEntry` ScriptableObject.
Use the following steps to create a new `RACMaterialEntry`:

1. Right-click in the Project window and select Create > Material.
    1. Adjust the properties in the Inspector window.
2. Right-click in the Project window and select Create > RoomAcoustiC++ > MaterialEntry.
3. Select the `RACMaterialEntry` and adjust its absorption parameters in the Inspector window. A number of preset materials are available from the drop-down menu.
4. Drag the Unity `Material` into the material slot of the `RACMaterialEntry` in the inspector to link the two together.

Each Unity `Material` should have a unique name and can be attached to any number of GameObjects.
Each `RACMaterialEntry` should have a unique linked Unity material.

## Assign material to geometry

To assign a material to geometry add the `RACMaterialEntry` to the materials list on the `RACMaterial` script.
Then, assign the linked Unity `Material` to the appropriate GameObjects in the scene.
Any geometry with a `Material` that is not linked to a `RACMaterialEntry` will be assigned default absorption properties.