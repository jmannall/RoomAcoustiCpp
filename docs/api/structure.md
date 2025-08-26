# Structure

All function calls from `interface.h` get passed to the `#!cpp Context` class which controls how this data is passed to other classes in RAC.

## Key Classes

- **Context**: Represents the main context for the RAC library, a singleton managing the overall state and configuration.
- **Room**: Stores the room geometry.
- **SourceManager**: Stores sound sources.
- **Source**: Represents a sound source within the room.
- **ImageSource**: Represents an image source, used for early reflections and diffraction.
- **ImageEdge**: Processes the image edge model.
- **Reverb**: Processes the FDN and reverb sources.

## Threads

The `#!cpp Context` creates a background thread that continuously runs the image edge model at 10ms intervals (or longer if processing exceeds 10ms).
When developing RAC, care must be taken to ensure thread safety between the background thread and function calls that will occur on the update thread (geometry and acoustic model configuration updates) and the audio thread.
As much as possible, the audio thread should be lock free and avoid the use of `std::mutex`.
Current usage of `#!cpp std::atomic<std::shared_ptr<>>`, which is not implemented by compilers in a lock free manner, mean some locks exist.