Functions for generating points on the surface of regular polyhedra (tetrahedron, octahedron, cube, icosahedron, dodecahedron).

- **Namespace:** `RAC::Common`
- **Header:** `Common/SphericalGeometries.h`
- **Source:** *(header only)*
- **Dependencies:** `Common/Types.h`, `Common/Vec3.h`

---

## Functions

### `#!cpp void Tetrahedron(std::vector<Vec3>& vertices, bool one)`
Appends the vertices of a tetrahedron to `vertices`.
- `vertices`: Output vector.
- `one`: If true, uses one orientation; otherwise, another.

---

### `#!cpp void Octahedron(std::vector<Vec3>& vertices)`
Appends the vertices of an octahedron to `vertices`.
- `vertices`: Output vector.

---

### `#!cpp void Cube(std::vector<Vec3>& vertices)`
Appends the vertices of a cube to `vertices`.
- `vertices`: Output vector.

---

### `#!cpp void Icosahedron(std::vector<Vec3>& vertices, bool one)`
Appends the vertices of an icosahedron to `vertices`.
- `vertices`: Output vector.
- `one`: If true, uses one orientation; otherwise, another.

---

### `#!cpp void Dodecahedron(std::vector<Vec3>& vertices, bool one)`
Appends the vertices of a dodecahedron to `vertices`.
- `vertices`: Output vector.
- `one`: If true, uses one orientation; otherwise, another.

---

## Implementation Notes

- Used for generating spherical sampling grids.
- Vertices are not normalized.

## Example Usage

```cpp
#include "Common/SphericalGeometries.h"
using namespace RAC::Common;

std::vector<Vec3> verts;
Tetrahedron(verts, true);
```
