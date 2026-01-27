# Structure

This section describes the structure of the RoomAcoustiC++ library, including its key components and how they interact.

## Key Classes

- **Context**: Represents the main context for the RoomAcoustiC++ library, managing the overall state and configuration.
- **DSPData / DSPConfig**: Stores initialization parameters and runtime DSP configuration flags (e.g., enable states and mode selection).
- **Room**: Stores the room geometry.
- **SourceManager**: Stores sound sources.
- **Source**: Represents a sound source within the room.
- **ImageSourceManager**: Manages a fixed pool of image sources used for early reflections and diffraction.
- **ImageSource**: Represents an image source used for early reflections and diffraction.
- **ImageEdge**: Runs the image edge model (IEM).
- **TracingThread**: Performs late reverberation ray tracing updates.
- **Reverb**: Base class for late reverberation processing (implemented by **SingleFDN** and **RAVES**).

The **RAVES** late reverb model implements MoD-ART, while the **SingleFDN** late reverb model (featured in the original version of RoomAcoustiC++) is closer to RAZR.