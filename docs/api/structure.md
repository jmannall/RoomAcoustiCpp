# Structure

This section describes the structure of the RoomAcoustiC++ library, including its key components and how they interact.

## Key Classes

- **Context**: Represents the main context for the RoomAcoustiC++ library, managing the overall state and configuration.
- **Room**: Stores the room geometry.
- **SourceManager**: Stores sound sources.
- **Source**: Represents a sound source within the room.
- **ImageSource**: Represents an image source used for early reflections and diffraction.
- **ImageEdge**: Processes the image edge model.
- **Reverb**: Processes the FDN and reverb sources.