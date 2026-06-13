# IIRFilter2Param1

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

The `IIRFilter2Param1` class implements a second-order IIR filter with a single interpolated parameter.
It derives from `IIRFilter2<In>` and is used as a base class for filters such as `LowPass`, `HighPass`, and the peak filters used in `GraphicEQ`.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/IIRFilter.h`
- **Source:** `DSP/IIRFilter.cpp`
- **Dependencies:** `Common/Types.h`, `DSP/Interpolate.h`

---

## Class Definition

```cpp
template<typename In = Real>
class IIRFilter2Param1 : public IIRFilter2<In>
{
public:
    IIRFilter2Param1(const Real parameter, const int sampleRate);
    virtual ~IIRFilter2Param1() {};

protected:
    inline void SetTargetParameter(const Real parameter);
    virtual void UpdateCoefficients(const Real parameter) = 0;

private:
    void InterpolateParameters(const Real lerpFactor) override;

    std::atomic<Real> target;
    Real current;
};
```

---

## Public Methods

### `#!cpp IIRFilter2Param1(const Real parameter, const int sampleRate)`
**Constructor.**  
Initialises the filter with the given control parameter and sample rate.

`parameter`: The initial parameter value.  
`sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp virtual ~IIRFilter2Param1()`
**Destructor.**  
Default destructor.

---

## Protected Methods

### `#!cpp inline void SetTargetParameter(const Real parameter)`
Sets the target parameter for the filter.

`parameter`: The new target parameter value.

---

### `#!cpp virtual void UpdateCoefficients(const Real parameter) = 0`
Updates the filter coefficients based on the current parameter.

Must be implemented in derived classes.

`parameter`: The control parameter.

---

## Private Methods

### `#!cpp void InterpolateParameters(const Real lerpFactor) override`
Interpolates between the current parameter and target parameter using linear interpolation.

`lerpFactor`: Interpolation factor (0.0 to 1.0).

---

## Internal Data Members

- `#!cpp std::atomic<Real> target`: Target filter parameter.
- `#!cpp Real current`: Current filter parameter.

---

## Implementation Notes

- Used as a base for filters with a single real-time interpolated parameter (e.g., cutoff frequency or gain).
