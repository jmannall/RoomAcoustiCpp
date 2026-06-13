The `IIRFilter`, `IIRFilter1`, and `IIRFilter2` classes implement Infinite Impulse Response (IIR) filters for digital signal processing.
They use a Direct-Form-II structure and support real-time parameter interpolation via derived classes.
These are base classes for multiple IIR filter implementations.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](../spatialiser/interface.md). This page documents lower-level details for advanced usage.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/IIRFilter.h`
- **Source:** `DSP/IIRFilter.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Coefficients.h`, `DSP/Buffer.h`

---

## Class Definitions

### `IIRFilter` (arbitrary order)

```cpp
class IIRFilter
{
public:
    IIRFilter(const int filterOrder, const int sampleRate);
    virtual ~IIRFilter() = default;

    Real GetOutput(const Real input, const Real lerpFactor);
    inline void ClearBuffers();
    Coefficients<> GetFrequencyResponse(const Coefficients<>& frequencies) const;
    bool IsValid() const;

protected:
    const int order;
    const Real T;

    Coefficients<> b;
    Coefficients<> a;
    Buffer<> y;

    std::atomic<bool> parametersEqual;
    std::atomic<bool> initialised;

private:
    virtual void InterpolateParameters(const Real lerpFactor) = 0;
};
```

---

### `IIRFilter1` (first order)

```cpp
class IIRFilter1
{
public:
    IIRFilter1(const int sampleRate);
    virtual ~IIRFilter1() = default;

    Real GetOutput(const Real input, const Real lerpFactor);
    inline void ClearBuffers();
    Coefficients<> GetFrequencyResponse(const Coefficients<>& frequencies) const;
    bool IsValid() const;

protected:
    const Real T;

    Real a0, a1;
    Real b0, b1;
    Real y0;

    std::atomic<bool> parametersEqual;
    std::atomic<bool> initialised;

private:
    virtual void InterpolateParameters(const Real lerpFactor) = 0;
};
```

---

### `IIRFilter2` (second order, templated)

```cpp
template<typename In = Real>
class IIRFilter2
{
public:
    IIRFilter2(const int sampleRate);
    virtual ~IIRFilter2() {};

    In GetOutput(const In input, const Real lerpFactor);
    void GetOutput(const In& input, In& output, const Real lerpFactor);

    inline void ClearBuffers();
    Coefficients<> GetFrequencyResponse(const Coefficients<>& frequencies) const;
    bool IsValid() const;

    static void GetOutputFromMultipleFilters(IIRFilter2** filters, int numFilters,
                                           const In& input, In& output, const Real lerpFactor);

protected:
    Real a1, a2;
    Real b1, b2;
    In   y0, y1;
    Real b0;

    const Real T;

    std::atomic<bool> parametersEqual;
    std::atomic<bool> initialised;

private:
    virtual void InterpolateParameters(const Real lerpFactor) = 0;
};
```

---

## Public Methods

### `#!cpp Real GetOutput(const Real input, const Real lerpFactor)`
Implemented by `IIRFilter` and `IIRFilter1`.

`input`: The input sample.  
`lerpFactor`: Interpolation factor used by derived classes for smoothing parameter changes.  
**Returns:** The filtered output sample.

---

### `#!cpp In GetOutput(const In input, const Real lerpFactor)`
Implemented by `IIRFilter2<In>`.

`input`: The input sample (defaults to `Real`, but may also be `Complex`).  
`lerpFactor`: Interpolation factor used by derived classes for smoothing parameter changes.  
**Returns:** The filtered output sample.

---

### `#!cpp void ClearBuffers()`
Resets the internal filter state (history) to zero.

---

### `#!cpp Coefficients<> GetFrequencyResponse(const Coefficients<>& frequencies) const`
Returns the filter response at the specified frequencies.

**Not thread-safe**; should only be called when `GetOutput` is not being called.

`frequencies`: Frequencies at which to calculate the response.  
**Returns:** The frequency response.

---

### `#!cpp bool IsValid() const`
Returns whether the filter has been initialised and can process audio.

---

## Implementation Notes

- These classes are intended to be subclassed; derived classes implement `InterpolateParameters(...)` and update coefficients.
- Parameter interpolation is designed for smooth transitions during real-time control.
