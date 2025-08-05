Manages thread-safe access to a resource, allowing or preventing access and tracking usage.

- **Namespace:** `RAC::Common`
- **Header:** `Common/Access.h`
- **Source:** *(header only)*
- **Dependencies:** `<atomic>`

---

## Class Definition

```cpp
class Access
{
public:
    Access();
    ~Access();

    inline bool CanEdit() const;

protected:
    inline bool GetAccess();
    inline void FreeAccess();
    inline void PreventAccess();
    inline void AllowAccess();

private:
    std::atomic<bool> accessFlag;
    std::atomic<int> inUse;
};
```

---

## Public Methods

### `#!cpp Access()`
**Default constructor.**  
Initializes the access control.

---

### `#!cpp ~Access()`
**Destructor.**  
Cleans up the access control.

---

### `#!cpp inline bool CanEdit() const`
Returns true if access is allowed and not in use.
- **Returns:** True if resource can be edited, false otherwise.

---

## Protected Methods

### `#!cpp inline bool GetAccess()`
Returns true if access is allowed and not in use, and marks as in use.
- **Returns:** True if access granted.

---

### `#!cpp inline void FreeAccess()`
Marks the resource as no longer in use.

---

### `#!cpp inline void PreventAccess()`
Prevents access to the resource.

---

### `#!cpp inline void AllowAccess()`
Allows access to the resource.

---

## Internal Data Members

- `#!cpp std::atomic<bool> accessFlag`: True if access is allowed.
- `#!cpp std::atomic<int> inUse`: Number of threads currently using the resource.

---

## Implementation Notes

- Intended for single-threaded access control.
- Uses atomic operations for thread safety.

## Example Usage

```cpp
#include "Common/Access.h"
using namespace RAC::Common;

class MyResource : public Access { /* ... */ };

MyResource res;
if (res.CanEdit()) {
    // Safe to edit
}
```
