Provides optional arguments for initialising the spatialiser context.

These parameters are optional and can be used to adjust logging and threading behaviour.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/ContextOptionalArguments.h`
- **Source:** *(header only)*
- **Dependencies:** *(standard library only)*

---

## Struct Definition

### `#!cpp struct ContextOptionalArguments`

**Fields:**

- `logPrefix`: prefix to add to any log file (default: empty)
- `desiredAudioThreads`: if set, overrides the number of audio threads to use

---

## Example Usage

```cpp
#include "Spatialiser/Interface.h"
#include "Spatialiser/ContextOptionalArguments.h"

using namespace RAC::Spatialiser;

DSPData data;
ContextOptionalArguments opt;
opt.logPrefix = "experimentA";
opt.desiredAudioThreads = 2;

bool ok = Init(data, opt);
```
