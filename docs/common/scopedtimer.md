A simple RAII timer for measuring code execution time, printing the result on destruction.

- **Namespace:** `RAC::Common`
- **Header:** `Common/ScopedTimer.h`
- **Source:** *(header only)*
- **Dependencies:** `<chrono>`, `<iostream>`, `<string>`

---

## Class Definition

```cpp
class ScopedTimer
{
public:
    ScopedTimer(std::ostream& outStream = std::cout);
    ~ScopedTimer();

    void Start();
    void Stop();
    void Stop(const std::string header);

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimePoint;
    std::ostream& m_outputStream;
};
```

---

## Public Methods

### `#!cpp ScopedTimer(std::ostream& outStream = std::cout)`
**Constructor.**  
Starts the timer and sets the output stream.
- `outStream`: Output stream for timing results.

---

### `#!cpp ~ScopedTimer()`
**Destructor.**  
Stops the timer and prints the elapsed time.

---

### `#!cpp void Start()`
Starts or restarts the timer.

---

### `#!cpp void Stop()`
Stops the timer and prints the elapsed time.

---

### `#!cpp void Stop(const std::string header)`
Stops the timer and prints the elapsed time with a header.
- `header`: String to print before the timing result.

---

## Internal Data Members

- `#!cpp std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimePoint`: Start time.
- `#!cpp std::ostream& m_outputStream`: Output stream.

---

## Implementation Notes

- Prints timing in milliseconds.
- Not thread-safe.

## Example Usage

```cpp
#include "Common/ScopedTimer.h"
using namespace RAC::Common;

{
    ScopedTimer timer;
    // ...code to time...
}
```
