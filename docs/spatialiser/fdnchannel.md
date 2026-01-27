Implements a single channel of a Feedback Delay Network (FDN), including a delay line and absorption filters.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](interface.md). This page documents lower-level details for advanced usage.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/FDN.h`
- **Source:** `Spatialiser/FDN.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Coefficients.h`, `DSP/Buffer.h`, `DSP/GraphicEQ.h`, `DSP/DelayLine.h`, `Spatialiser/Configs.h`

---

## Class Definition

```cpp
template <typename T = Real>
class FDNChannel
{
public:
    // Real-valued (banded) channel
    FDNChannel(const int delayLength, const Coefficients<>& T60, const std::shared_ptr<DSPConfig>& dspConfig);

    // Complex-valued (MoD-ART) channel
    FDNChannel(const int delayLength, const Real T60, const std::shared_ptr<DSPConfig>& dspConfig);

    ~FDNChannel();

    inline void SetTargetT60(const Coefficients<>& T60);
    inline bool SetTargetReflectionFilter(const Coefficients<>& gains);
    inline void Reset();

    inline void ProcessOutput(const Buffer<T>& data, Buffer<T>& outputBuffer, const Real lerpFactor);

    void GetOutput(T& output, const T& input, const Real lerpFactor);
    inline T GetOutput(const T& input, const Real lerpFactor);

private:
    inline Coefficients<> CalculateFilterGains(const Coefficients<>& T60) const;
    inline Real CalculateFilterGains(const Real T60) const;

    const Real mT;
    DelayLine<T> mDelayLine;

    // Real: GraphicEQ absorption + reflection filters
    // Complex: scalar absorption gain
    std::conditional_t<std::is_same_v<T, Real>, GraphicEQ<Real>, std::atomic<Real>> mAbsorptionFilter;
    std::conditional_t<std::is_same_v<T, Real>, GraphicEQ<Real>, std::nullptr_t> mReflectionFilter;
};
```

---

## Public Methods

### `#!cpp FDNChannel(const int delayLength, const Coefficients<>& T60, const std::shared_ptr<DSPConfig>& dspConfig)`
**Constructor (Real).**  
Initialises a banded (real-valued) channel with a delay line and absorption filter.

`delayLength`: Delay in samples.  
`T60`: Target decay time per frequency band.  
`dspConfig`: DSP configuration.

---

### `#!cpp FDNChannel(const int delayLength, const Real T60, const std::shared_ptr<DSPConfig>& dspConfig)`
**Constructor (Complex).**  
Initialises a complex-valued channel with a delay line and scalar absorption gain.

`delayLength`: Delay in samples.  
`T60`: Target decay time in seconds.  
`dspConfig`: DSP configuration.

---

### `#!cpp ~FDNChannel()`
Destroys the channel instance.

---

### `#!cpp inline void SetTargetT60(const Coefficients<>& T60)`
Sets the target T60 and updates the absorption filter gains (real-valued channels).

`T60`: New target decay time.

---

### `#!cpp inline bool SetTargetReflectionFilter(const Coefficients<>& gains)`
Sets the target reflection filter gains (real-valued channels).

`gains`: Target reflection filter gains.  
**Returns:** True if all current and target reflection gains are zero, false otherwise.

---

### `#!cpp inline void ProcessOutput(const Buffer<T>& data, Buffer<T>& outputBuffer, const Real lerpFactor)`
Processes the reflection filter on the channel output (real-valued channels).

`data`: Input audio data.  
`outputBuffer`: Output buffer.  
`lerpFactor`: Linear interpolation factor.

---

### `#!cpp void GetOutput(T& output, const T& input, const Real lerpFactor)`
Processes a single-sample output from the delay line and absorption stage.

`output`: Processed output sample.  
`input`: Next sample to feed into the delay line.  
`lerpFactor`: Linear interpolation factor.

---

### `#!cpp inline void Reset()`
Resets the internal delay line (and clears filter state, where applicable).

---

### `#!cpp inline T GetOutput(const T& input, const Real lerpFactor)`
Convenience overload returning the processed sample.

`input`: Next sample to feed into the delay line.  
`lerpFactor`: Linear interpolation factor.  
**Returns:** The processed output sample.


## Private Methods


### `#!cpp inline Coefficients<> CalculateFilterGains(const Coefficients<>& T60) const`
Computes per-band absorption filter gains required to match the target decay time (real-valued channels).

`T60`: Target decay time per frequency band.  
**Returns:** Filter gain coefficients.

---

### `#!cpp inline Real CalculateFilterGains(const Real T60) const`
Computes the scalar absorption gain required to match the target decay time (complex-valued channels).

`T60`: Target decay time in seconds.  
**Returns:** Gain value.
