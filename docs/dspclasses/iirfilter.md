The `IIRFilter1` and `IIRFilter2` classes implement a first or second order Infinite Impulse Response (IIR) filter for digital signal processing.
It uses the Direct-Form-II structure and supports real-time parameter interpolation.
It is the base class for multiple IIR filter implementations.

- **Namespace:** `RAC::DSP`
- **Header:** `DSP/IIRFilter.h`
- **Source:** `DSP/IIRFilter.cpp`
- **Dependencies:** `Common/Types.h`, `Coefficients` (type for frequency response)

---

## Class Definition

``` cpp title="" linenums="1"
class IIRFilter2
{
    public:
        IIRFilter2(const int sampleRate);
        ~IIRFilter2();

        Real GetOutput(const Real input, const Real lerpFactor);
        inline void ClearBuffers();
        Coefficients GetFrequencyResponse(const Coefficients& frequencies) const

    protected:
        virtual void InterpolateParameters(const Real lerpFactor) = 0

        const Real T;
        Real a0, a1, a2;
        Real b0, b1, b2;
        Real y0, y1;
        std::atomic<bool> parametersEqual;
        std::atomic<bool> initialised;

    private:
        std::atomic<bool> clearBuffers;
};
```

---

## Public Methods

### `#!cpp IIRFilter2(const int sampleRate)`
**Constructor.**  
Initializes the IIR filter with the given sample rate.  
- `sampleRate`: The sample rate for calculating filter coefficients.

---

### `#!cpp ~IIRFilter2()`
**Destructor.**  
Cleans up the filter.

---

### `#!cpp Real GetOutput(const Real input, const Real lerpFactor)`
Processes a single input sample and returns the filtered output.  
- If the filter is not initialized, returns 0.0.
- If the filter parameters are not equal to the target, calls `InterpolateParameters`.
- If the `clearBuffers` flag is set, zeros the output buffers before processing.
- `input`: The input sample.
- `lerpFactor`: The lerp factor for interpolation between current and target filter parameters.
- **Returns:** The filtered output sample.

---

### `#!cpp inline void ClearBuffers()`
Sets `clearBuffers` to true, which will zero the output buffers on the next call to `GetOutput`.

---

### `#!cpp Coefficients GetFrequencyResponse(const Coefficients& frequencies) const`
Returns the filter's frequency response at the specified frequencies.
**Not thread-safe**; should only be called when `GetOutput` is not being called.
- `frequencies`: The frequencies at which to calculate the response.
- **Returns:** The frequency response of the filter.

---

## Protected Methods

### `#!cpp virtual void InterpolateParameters(const Real lerpFactor) = 0`
Pure virtual function for linearly interpolating filter parameters.  
- Must be implemented in derived classes.
- `lerpFactor`: The lerp factor for interpolation.

---

## Internal Data Members

- `#!cpp const Real T`: The sample period (1/sampleRate).
- `#!cpp Real a0, a1, a2`: Denominator coefficients (audio thread only).
- `#!cpp Real b0, b1, b2`: Numerator coefficients (audio thread only).
- `#!cpp Real y0, y1`: Output buffer (audio thread only).
- `#!cpp std::atomic<bool> parametersEqual`: True if current parameters are equal to target.
- `#!cpp std::atomic<bool> initialised`: True if the filter is initialized.
- `#!cpp std::atomic<bool> clearBuffers`: Flag to clear output buffers.

---

## Implementation Notes

- The filter uses Direct-Form-II for efficient processing.
- Parameter interpolation is supported for smooth transitions between filter states.
- Buffer clearing is managed via an atomic flag for real-time safety.
- The class is intended to be subclassed, with parameter interpolation logic implemented in derived classes.
