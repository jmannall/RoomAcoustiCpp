# Changing the RoomAcoustiC++ plugin

Four different plugins are provided for the RoomAcoustiC++ acoustic model.
They are controlled through a compiler flag that can be automatically assigned using the `RACManager` script.
Simply select the desired plugin in the `RACManager` inspector and press **Apply Plugin**.
This will update the compiler flag and rebuild the project with the selected plugin.

- **Default**: The default plugin that should be used for any final application (RAC_DEFAULT).
- **Debug**: Enables debug messages in the Unity console, visualing the acoustic model in the Unity scene and measuring impulse responses (RAC_DEBUG).
- **Profile**: Logs performance metrics for the audio and background thread to a text file (RAC_PROFILE).
- **Profile detailed**: Logs detailed performance metrics for subtasks on audio and background thread to a text file (RAC_PROFILE_DETAILED).

The drop down will always revert to Default when the project is loaded and when the `RACManager` script is created.
Therefore, it should not be relied upon to always show the current active plugin.

## Changing the compiler flag manually

To change the compiler flag manually, or to see the currently selected plugin go to Edit > Project Settings > Player and look for the "Scripting Define Symbols" field.