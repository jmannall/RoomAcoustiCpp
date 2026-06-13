Stores frequency-dependent **absorption coefficients** for surface materials.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

Absorption in RoomAcoustiC++ is represented using `Coefficients<>` (one value per frequency band), where each value is in the range **[0, 1]**.

- **Namespace:** `RAC::Common`
- **Type:** `Coefficients<>`
- **Header:** `Common/Coefficients.h`
- **Source:** *(header only)*
- **Dependencies:** `Common/Types.h`, `Common/Definitions.h`

---

## Absorption Coefficients

An absorption coefficient describes how much energy is absorbed by a surface at a given frequency band:

- `0.0` means **fully reflective** (no energy is lost)
- `1.0` means **fully absorbing** (no energy is reflected)

In the public API, absorption coefficients are used when creating **materials** (see `RAC::Spatialiser::InitMaterial`) and are referenced by geometry such as walls.

---

## Example Usage

```cpp
#include "Common/Coefficients.h"
#include "Spatialiser/Interface.h"

using namespace RAC::Common;
using namespace RAC::Spatialiser;

// 4-band absorption (e.g., 250, 500, 1000, 2000 Hz)
Coefficients<> absorption(std::vector<Real>({0.2, 0.3, 0.4, 0.5}));

// Create a material from absorption coefficients
int materialId = InitMaterial(absorption);
```
