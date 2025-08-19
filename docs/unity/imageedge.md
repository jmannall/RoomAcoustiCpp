# Configure the acoustic model

The acoustic model can be configured in the `RACManager` script. For an overview of the acoustic model see the [documentation of the acoustic model](../overview/model.md).

## Configuration

The acoustic model can be configured with the following parameters:

- **Direct sound**: None, Check (Checks for obstructions), Always On (Does not check for obstructions).
- **Reflection Order**: The maximum order paths including only reflections.
- **Shadow Diffraction Order**: The maximum order of paths including shadowed diffraction.
- **Specular Diffraction Order**: The maximum order of paths including non-shadowed edge diffraction.
- **Late Reverb**: If true, late reverberation is enabled.
- **Minimum Edge Length**: The minimum length of edges to consider for diffraction.

It is possible to configure the image edge model such that too many image sources are created than can be processed in a single audio buffer.
This can lead to buffer underruns resulting in audible artifacts.
This is dependent on the computer hardware and the complexity of the scene.
If this occurs, it may be necessary reduce the maximum order of the image edge model.