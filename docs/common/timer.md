Implements a thread that periodically calls a callback function at a specified interval.

- **Namespace:** *(global)*
- **Header:** `Common/Timer.h`
- **Source:** *(header only)*
- **Dependencies:** `<thread>`, `<atomic>`, `<chrono>`, `<functional>`

---

## Class Definition

```cpp
class Timer
{
public:
    Timer();
    virtual ~Timer();

    void StartTimer(int intervalMs);
    void StopTimer();

protected:
    virtual void TimerCallback() = 0;

private:
    std::thread timerThread;
    std::atomic<bool> running;
};
```

---

## Public Methods

### `#!cpp Timer()`
**Default constructor.**  
Initializes the timer.

---

### `#!cpp virtual ~Timer()`
**Destructor.**  
Stops the timer.

---

### `#!cpp void StartTimer(int intervalMs)`
Starts the timer with a callback interval in milliseconds.
- `intervalMs`: Interval in milliseconds.

---

### `#!cpp void StopTimer()`
Stops the timer.

---

## Protected Methods

### `#!cpp virtual void TimerCallback() = 0`
Callback function called every interval.

---

## Internal Data Members

- `#!cpp std::thread timerThread`: Thread running the timer.
- `#!cpp std::atomic<bool> running`: True if timer is running.

---

## Implementation Notes

- Used as a base for periodic background tasks.
- Not copyable or movable.

## Example Usage

```cpp
#include "Common/Timer.h"

class MyTimer : public Timer {
protected:
    void TimerCallback() override {
        // Do something
    }
};

MyTimer t;
t.StartTimer(1000); // 1 second interval
```
