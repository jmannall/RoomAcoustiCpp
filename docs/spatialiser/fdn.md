Implements a Feedback Delay Network (FDN) for artificial reverberation, with configurable channel delays, absorption, and reflection filters.  
Supports custom feedback matrices and room configurations.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/FDN.h`
- **Source:** `Spatialiser/FDN.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Matrix.h`, `Common/Vec.h`, `Common/Coefficients.h`, `DSP/Buffer.h`, `DSP/GraphicEQ.h`, `Spatialiser/Types.h`

---

## Class Definition

```cpp
class FDN
{
public:
    FDN(const Coefficients& T60, const Vec& dimensions, const Config& config);
    virtual ~FDN();

    void SetTargetT60(const Coefficients& T60);
    inline bool SetTargetReflectionFilters(const std::vector<Absorption>& gains);
    void ProcessAudio(const Matrix& data, std::vector<Buffer>& outputBuffers);
    inline void Reset();

protected:
    FDN(const Coefficients& T60, const Vec& dimensions, const Config& config, const Matrix& matrix);
    void ProcessSquare();

    Rowvec x;
    Rowvec y;

private:
    std::vector<int> CalculateTimeDelay(const Vec& dimensions);
    static inline Matrix InitMatrix(const size_t numChannels);
    virtual inline void ProcessMatrix();
    static bool IsSetMutuallyPrime(const std::vector<int>& numbers);
    static bool IsEntryMutuallyPrime(const std::vector<int>& numbers, int idx);
    static void MakeSetMutuallyPrime(std::vector<int>& numbers);

    const Matrix feedbackMatrix;
    Config mConfig;
    std::vector<std::unique_ptr<FDNChannel>> mChannels;
    std::atomic<bool> clearBuffers;
};
```

---

## Public Methods

### `#!cpp FDN(const Coefficients& T60, const Vec& dimensions, const Config& config)`
**Constructor.**  
Initializes an FDN with target T60 and room dimensions.
- `T60`: Target decay time.
- `dimensions`: Room dimensions (determines delay line lengths).
- `config`: Spatialiser configuration.

---

### `#!cpp virtual ~FDN()`
**Destructor.**  
Cleans up the FDN.

---

### `#!cpp void SetTargetT60(const Coefficients& T60)`
Updates the target T60 for all channels.
- `T60`: New decay time.

---

### `#!cpp inline bool SetTargetReflectionFilters(const std::vector<Absorption>& gains)`
Sets the target reflection filters for each channel.
- `gains`: Target reflection filter gains for each channel.
- **Returns:** True if all channels have zero target reflection gains, false otherwise.

---

### `#!cpp void ProcessAudio(const Matrix& data, std::vector<Buffer>& outputBuffers)`
Processes a multichannel audio buffer through the FDN.
- `data`: Multichannel input (numChannels x numFrames).
- `outputBuffers`: Output buffers.

---

### `#!cpp inline void Reset()`
Resets all internal FDN buffers to zero.

---

## Protected Methods

### `#!cpp FDN(const Coefficients& T60, const Vec& dimensions, const Config& config, const Matrix& matrix)`
Protected constructor for custom feedback matrices.

---

### `#!cpp void ProcessSquare()`
Processes a square feedback matrix.

---

## Private Methods

### `#!cpp std::vector<int> CalculateTimeDelay(const Vec& dimensions)`
Calculates sample delays for each FDN channel based on room dimensions.
- `dimensions`: Room dimensions.
- **Returns:** Delay lengths for each channel.

---

### `#!cpp static inline Matrix InitMatrix(const size_t numChannels)`
Initializes a default diagonal matrix.
- `numChannels`: Number of channels.
- **Returns:** Diagonal matrix.

---

### `#!cpp virtual inline void ProcessMatrix()`
Runs the currently selected matrix process function.

---

### `#!cpp static bool IsSetMutuallyPrime(const std::vector<int>& numbers)`
Checks if a set of numbers is mutually prime.

---

### `#!cpp static bool IsEntryMutuallyPrime(const std::vector<int>& numbers, int idx)`
Checks if a single entry is mutually prime with all others.

---

### `#!cpp static void MakeSetMutuallyPrime(std::vector<int>& numbers)`
Makes a set of numbers mutually prime by adjusting entries.

---

## Internal Data Members

- `#!cpp const Matrix feedbackMatrix`: Feedback matrix.
- `#!cpp Config mConfig`: Spatialiser configuration.
- `#!cpp std::vector<std::unique_ptr<FDNChannel>> mChannels`: Delay line channels.
- `#!cpp std::atomic<bool> clearBuffers`: Flag to clear buffers.
- `#!cpp Rowvec x`: Input buffer.
- `#!cpp Rowvec y`: Output buffer.

---

## Implementation Notes

- FDN supports custom feedback matrices and ensures mutually prime delay lengths for decorrelation.
- HouseHolderFDN and RandomOrthogonalFDN are derived classes for specific matrix types.
- Thread-safe buffer clearing and parameter updates.

## Example Usage

```cpp
#include "Spatialiser/FDN.h"
using namespace RAC::Spatialiser;

Coefficients T60 = { 1.2, 1.0, 0.8 };
Vec dimensions = { 5.0, 4.0, 3.0 };
Config config;
config.fs = 48000;
config.numLateReverbChannels = 3;
config.numFrames = 512;

// Create FDN
FDN fdn(T60, dimensions, config);

// Set new T60
fdn.SetTargetT60({ 1.0, 0.9, 0.7 });

// Process audio
Matrix input(config.numLateReverbChannels, config.numFrames);
std::vector<Buffer> output(config.numLateReverbChannels, Buffer(config.numFrames));
fdn.ProcessAudio(input, output);
```
