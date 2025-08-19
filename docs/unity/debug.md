# Receive debug messages and visualise the acoustic model

The `DebugCpp` script can be used to receive additional debug messages from RoomAcoustiC++ and to visualise the acoustic model in the Unity scene.
It can only be used with the **Debug** plugin.
See [Changing the RoomAcoustiC++ plugin](plugin.md) for details on how to change the active RoomAcoustiC++ plugin.

To enable debug messages, add the `DebugCpp` script to an empty GameObject in the Unity scene.
In order to visualise the acoustic model drag the desired `RACAudioSource` to the source field of the `DebugCpp` script and enable gizmos by clicking the top right button in the scene view.
When you play the scene you should see the acoustic model visualised in the scene view and the location of any discovered edges in the scene geometry.