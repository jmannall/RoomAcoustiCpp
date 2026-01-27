Implements a Feedback Delay Network (FDN) for artificial reverberation, with configurable channel delays, absorption, and reflection filters.

Most users will configure late reverberation through the main API in `Spatialiser/Interface.h` (e.g., `InitSingleFDN`). The FDN classes are exposed for advanced usage.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/FDN.h`
- **Source:** `Spatialiser/FDN.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Matrix.h`, `Common/Vec.h`, `Common/Coefficients.h`, `DSP/Buffer.h`, `DSP/GraphicEQ.h`, `Spatialiser/Types.h`, `Spatialiser/Configs.h`

---

## Class Definition

```cpp
template <typename T = Real>
class FDN
{
public:
    // Real-valued (banded) FDN
    FDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig>& dspConfig);

    // Complex-valued (MoD-ART) FDN
    FDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig>& dspConfig);

    virtual ~FDN();

    // Real-valued controls
    inline void SetTargetT60(const Coefficients<>& T60);
    inline bool SetTargetReflectionFilters(const std::vector<Coefficients<>>& gains);

    // Complex-valued (MoD-ART) controls
    inline void SetTargetResidues(const Coefficients<>& residues);
    inline void SetPrecedingDelay(const int samples);
    inline void SetMinimumReverbTime(Real T60);

    // Complex-valued input submission
    // NOTE: The exact SubmitAudio signature depends on the build configuration.
    // One of the following overloads will be available:
    inline void SubmitAudio(const Vec<Real>& input);
    inline void SubmitAudio(const Matrix<>& input, int row);


    // Processing
    void ProcessAudio(const Matrix<>& data, std::vector<Buffer<>>& outputBuffers, const AudioData& audioData);
    void ProcessAudio(std::vector<Buffer<>>& outputBuffers, const AudioData& audioData);

protected:
    FDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig> dspConfig, const Matrix<>& matrix);
    FDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig> dspConfig, const Matrix<>& matrix);

    Rowvec<T> x;
    Vec<T> y;

private:
    // Internal helpers
    inline void Reset();
    virtual inline void ProcessMatrix();
    void ProcessSquare();

    Vec<int> CalculateTimeDelay(const Vec<>& dimensions, const int fdnSize, const int fs);
    static inline Matrix<> InitMatrix(const size_t numChannels);

    static bool IsSetMutuallyPrime(const Vec<int>& numbers);
    static bool IsEntryMutuallyPrime(const Vec<int>& numbers, int idx);
    static void MakeSetMutuallyPrime(Vec<int>& numbers);

    const Matrix<> feedbackMatrix;
    std::vector<std::unique_ptr<FDNChannel<T>>> mChannels;
};
```

---

## Public Methods

### `#!cpp FDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig>& dspConfig)`
**Constructor (Real).**  
Initialises a banded (real-valued) FDN with a target T60 and room dimensions.

`T60`: Target decay time per frequency band.  
`dimensions`: Room dimensions (used to derive delay lengths).  
`dspConfig`: DSP configuration.

---

### `#!cpp FDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig>& dspConfig)`
**Constructor (Complex).**  
Initialises a complex-valued FDN using explicit delay lengths.

`T60`: Target decay time (seconds).  
`delayLengths`: Delay line lengths (in samples).  
`dspConfig`: DSP configuration.

---

### `#!cpp virtual ~FDN()`
Destroys the FDN instance and releases any owned resources.

---

### `#!cpp inline void SetTargetT60(const Coefficients<>& T60)`
Updates the target T60 for all channels.

`T60`: New decay time per frequency band.

---

### `#!cpp inline void SetTargetResidues(const Coefficients<>& residues)`
Updates the per-output residues used by the complex-valued (MoD-ART) processing.

`residues`: New residues (one per output channel).

---

### `#!cpp inline bool SetTargetReflectionFilters(const std::vector<Coefficients<>>& gains)`
Sets the target reflection filters for each channel.

`gains`: Target reflection filter gains for each channel.  
**Returns:** True if all channels have zero target reflection gains, false otherwise.

---

### `#!cpp inline void SetPrecedingDelay(const int samples)`
Sets a *preceding delay* (in samples) used by the complex-valued (MoD-ART) processing path.

`samples`: Delay length in samples.


---

### `#!cpp inline void SetMinimumReverbTime(Real T60)`
Enables or disables the complex-valued (MoD-ART) processing depending on the configured minimum reverberation time.

`T60`: Minimum reverberation time (seconds).


---

### `#!cpp inline void SubmitAudio(const Vec<Real>& input)`
Submits complex-path input audio.

**Note:** Depending on the build configuration, complex input submission is provided either via this overload or via the `Matrix<>` overload below.

`input`: Input samples for the current callback.


---

### `#!cpp inline void SubmitAudio(const Matrix<>& input, int row)`
Submits complex-path input audio from a single row of a matrix.

**Note:** Depending on the build configuration, complex input submission is provided either via the `Vec<Real>` overload above or via this overload.

`input`: Input matrix containing audio samples.  
`row`: Row index to read from.


---

### `#!cpp void ProcessAudio(const Matrix<>& data, std::vector<Buffer<>>& outputBuffers, const AudioData& audioData)`
Processes a multichannel audio buffer through the FDN.

`data`: Multichannel input (numChannels x numFrames).  
`outputBuffers`: Output buffers to write to.  
`audioData`: Per-callback audio configuration (e.g., interpolation factor and mode flags).

---

### `#!cpp void ProcessAudio(std::vector<Buffer<>>& outputBuffers, const AudioData& audioData)`
Processes audio through the complex-valued (MoD-ART) FDN.

`outputBuffers`: Output buffers to accumulate into.  
`audioData`: Per-callback audio configuration (e.g., interpolation factor and mode flags).


---


## Protected Methods

### `#!cpp FDN(const Coefficients<>& T60, const Vec<>& dimensions, const std::shared_ptr<DSPConfig> dspConfig, const Matrix<>& matrix)`
Protected constructor used by derived FDN types to supply a custom feedback matrix (real-valued path).

`T60`: Target decay time per frequency band.  
`dimensions`: Room dimensions.  
`dspConfig`: DSP configuration.  
`matrix`: Feedback matrix to use.

---

### `#!cpp FDN(const Real T60, const Vec<int>& delayLengths, const std::shared_ptr<DSPConfig> dspConfig, const Matrix<>& matrix)`
Protected constructor used by derived FDN types to supply a custom feedback matrix (complex-valued path).

`T60`: Target decay time (seconds).  
`delayLengths`: Delay line lengths (in samples).  
`dspConfig`: DSP configuration.  
`matrix`: Feedback matrix to use.

---

### `#!cpp void ProcessSquare()`
Processes the default square feedback matrix operation (used by the base implementation of `ProcessMatrix()`).


## Private Methods

### `#!cpp inline void Reset()`
Clears internal delay lines and filter buffers. This is used internally by the FDN (not part of the public interface).

---

### `#!cpp virtual inline void ProcessMatrix()`
Processes the feedback matrix for the FDN.

The base implementation calls `ProcessSquare()`. Derived classes may override this to apply alternative matrices (e.g., Householder or random orthogonal).

---

### `#!cpp Vec<int> CalculateTimeDelay(const Vec<>& dimensions, const int fdnSize, const int fs)`
Computes a set of delay line lengths (in samples) from room dimensions and configuration.

`dimensions`: Room dimensions.  
`fdnSize`: Number of channels in the FDN.  
`fs`: Sample rate.  
**Returns:** Delay line lengths (samples).

---

### `#!cpp static inline Matrix<> InitMatrix(const size_t numChannels)`
Initialises the default feedback matrix for an FDN of the given size.

`numChannels`: Number of FDN channels.  
**Returns:** Feedback matrix.

---

### `#!cpp static bool IsSetMutuallyPrime(const Vec<int>& numbers)`
Checks whether all numbers in the set are mutually prime.

`numbers`: The set of integers to test.  
**Returns:** True if the set is mutually prime, false otherwise.

---

### `#!cpp static bool IsEntryMutuallyPrime(const Vec<int>& numbers, int idx)`
Checks whether a single entry is mutually prime with all other entries in the set.

`numbers`: The set of integers to test.  
`idx`: Index of the entry to check.  
**Returns:** True if mutually prime, false otherwise.

---

### `#!cpp static void MakeSetMutuallyPrime(Vec<int>& numbers)`
Adjusts a set of integers to be mutually prime.

`numbers`: The set of integers to modify.

---

## Example Usage

```cpp
#include "Spatialiser/Configs.h"
#include "Spatialiser/FDN.h"

using namespace RAC::Spatialiser;

DSPData dsp;
std::shared_ptr<DSPConfig> dspConfig = std::make_shared<DSPConfig>(dsp);

// Target T60 per band (must match the configured band count)
Coefficients<> T60 = Coefficients<>::Constant(dsp.frequencyBands.Length(), 1.0);

// Room dimensions (m)
Vec<> dims(std::vector<Real>({ 5.0, 4.0, 3.0 }));

FDN<Real> fdn(T60, dims, dspConfig);

// Per-callback audio flags
AudioData audioData(dspConfig);

Matrix<> input(dspConfig->GetData().fdnSize, dspConfig->GetData().numFrames);
std::vector<Buffer<>> output(dspConfig->GetData().numReverbSources, Buffer<>(dspConfig->GetData().numFrames));

fdn.ProcessAudio(input, output, audioData);
```
