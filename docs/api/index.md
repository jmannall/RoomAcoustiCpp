# Overview

The C++ API provides a set of functions for using the RoomAcoustiC++ library in your own applications.
It is designed to be easy to use and integrate with existing C++ codebases.
It is available from the [RoomAcoustiCpp](https://github.com/jmannall/RoomAcoustiCpp) repository.

The following sections outline the interface structure and configuration types required to use RoomAcoustiC++ in your own applications.

## Getting started (Recommended for first time users)

- [Installation](installation.md): How to add RoomAcoustiCpp to your existing project.
- [Interface](interface.md): An overview of the RAC C++ interface and available function calls.

```bash
git clone https://github.com/jmannall/RoomAcoustiCpp.git
```

### For contributors
- [Structure](structure.md): A summary of the main classes used within RAC.

## Reference

- `Spatialiser/Interface.h`: main API entry point
- `Spatialiser/Types.h`: enums and typedefs used by the API
- `Spatialiser/Configs.h`: configuration structs
- `Spatialiser/ContextOptionalArguments.h`: optional initialisation parameters
