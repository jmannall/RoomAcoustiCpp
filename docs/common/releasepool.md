Garbage collector for shared pointers, periodically erasing unreferenced objects.

- **Namespace:** *(global)*
- **Header:** `Common/ReleasePool.h`
- **Source:** *(header only)*
- **Dependencies:** `Common/Timer.h`, `<vector>`, `<mutex>`

---

## Class Definition

```cpp
class ReleasePool : private Timer
{
public:
    ReleasePool();
    ~ReleasePool();

    template<typename T>
    void Add(const std::shared_ptr<T>& object);

private:
    void TimerCallback() override;

    std::vector<std::shared_ptr<void>> pool;
    std::mutex m;
};
```

---

## Public Methods

### `#!cpp ReleasePool()`
**Constructor.**  
Starts a timer to periodically clean up unreferenced shared pointers.

---

### `#!cpp ~ReleasePool()`
**Destructor.**  
Stops the timer.

---

### `#!cpp template<typename T> void Add(const std::shared_ptr<T>& object)`
Adds a shared pointer to the pool.
- `object`: Shared pointer to add.

---

## Private Methods

### `#!cpp void TimerCallback() override`
Called every second to erase unreferenced shared pointers.

---

## Internal Data Members

- `#!cpp std::vector<std::shared_ptr<void>> pool`: Pool of shared pointers.
- `#!cpp std::mutex m`: Mutex for thread safety.

---

## Implementation Notes

- Based on CppCon 2015: Timur Doumler "C++ in the Audio Industry".
- Used for lock-free atomic pointer replacement in real-time audio.

## Example Usage

```cpp
#include "Common/ReleasePool.h"

ReleasePool pool;
pool.Add(std::make_shared<int>(42));
```
