# IIRFilter2Param1

The `IIRFilter2Param1` class implements a second-order IIR filter with a single interpolated parameter.
It derives from `IIRFilter2`.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/IIRFilter.h`
- **Source:** `DSP/IIRFilter.cpp`
- **Dependencies:** `Common/Types.h`, `DSP/Interpolate.h`

---
## Class Definition

```cpp
class IIRFilter2Param1 : public IIRFilter2
{
public:
    IIRFilter2Param1(const Real parameter, const int sampleRate);
    virtual ~IIRFilter2Param1();
    // ...inherited methods from IIRFilter2...

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
Initializes the filter with the given sample rate and parameter.
- `parameter`: The initial parameter value.
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp virtual ~IIRFilter2Param1()`
**Destructor.**  
Cleans up the filter.

---

## Protected Methods

### `#!cpp inline void SetTargetParameter(const Real parameter)`
Sets the target parameter for the filter.
- `parameter`: The new target parameter value.

---

### `#!cpp virtual void UpdateCoefficients(const Real parameter)`
Updates the filter coefficients based on the current parameter.
- Must be implemented in derived classes.
- `parameter`: The parameter value.

---

## Private Methods

### `#!cpp void InterpolateParameters(const Real lerpFactor)`
Interpolates between the current filter parameter and target filter parameter using linear interpolation.
- `lerpFactor`: Interpolation factor (0.0 to 1.0)

---

## Internal Data Members

- `#!cpp std::atomic<Real> target`: Target filter parameter
- `#!cpp Real current`: Current filter parameter

---

## Implementation Notes

- Used as a base for filters with a single real-time interpolated parameter (e.g., cutoff frequency or gain).
