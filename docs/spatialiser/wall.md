Implements the `Wall` and `Plane` classes for geometric room modeling, including wall/plane intersection and reflection utilities.

Most users will interact with RoomAcoustiC++ through the high-level API in [`Spatialiser/Interface.h`](interface.md). This page documents lower-level details for advanced usage.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/Wall.h`
- **Source:** `Spatialiser/Wall.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Vec3.h`, `Common/Coefficients.h`, `Spatialiser/Edge.h`, `Spatialiser/Types.h`

---

## Utility Functions

### `#!cpp std::pair<bool, Vec3> IntersectTriangle(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& origin, const Vec3& dir, const bool returnIntersection)`
Determines whether a line intersects a triangle.

`v1`, `v2`, `v3`: Triangle vertices.  
`origin`: Line origin.  
`dir`: Line direction.  
`returnIntersection`: If true, returns the intersection point; otherwise only performs the intersection test.  
**Returns:** A pair containing whether an intersection occurred and (optionally) the intersection point.

---

## Class Definition

```cpp
class Wall
{
public:
    Wall(const Vertices& vData, size_t materialId);
    ~Wall();

    inline void AddEdge(const size_t id);
    inline void RemoveEdge(const size_t id);
    inline bool EmptyEdges() const;
    inline Vec3 GetNormal() const;
    inline Real GetD() const;
    inline size_t GetMaterialID() const;
    inline Vertices GetVertices() const;
    inline bool VertexMatch(const Vec3& x) const;
    inline Real GetArea() const;
    inline std::vector<size_t> GetEdges() const;
    inline size_t GetPlaneID() const;
    inline void SetPlaneID(const size_t id);
    inline Real PointWallPosition(const Vec3& point) const;
    bool LineWallIntersection(const Vec3& start, const Vec3& end, Vec3& intersection) const;
    inline bool LineWallObstruction(const Vec3& start, const Vec3& end) const;
    void Update(const Vertices& vData);

private:
    inline void CalculateArea();

    Vertices mVertices;
    Vec3 mNormal;
    Real d;
    Real area;

    size_t mPlaneId;
    size_t materialId;
    std::vector<size_t> mEdges;
};

class Plane
{
public:
    Plane();
    Plane(const size_t id, const Wall& wall);
    ~Plane();

    inline void AddWall(const size_t id);
    inline bool RemoveWall(const size_t id);
    inline Vec3 GetNormal() const;
    inline Real GetD() const;
    const inline std::vector<size_t>& GetWalls() const;
    inline bool GetReceiverValid() const;
    inline void SetReceiverValid(const Vec3& listenerPosition);
    inline bool IsCoplanar(const Wall& wall) const;
    inline Real PointPlanePosition(const Vec3& point) const;
    bool LinePlaneObstruction(const Vec3& start, const Vec3& end) const;
    bool LinePlaneIntersection(const Vec3& start, const Vec3& end) const;
    bool ReflectPointInPlane(const Vec3& point) const;
    bool ReflectPointInPlane(Vec3& dest, const Vec3& point) const;
    void ReflectPointInPlaneNoCheck(Vec3& point) const;
    void ReflectNormalInPlane(Vec3& normal) const;
    bool EdgePlanePosition(const Edge& edge) const;
    inline void Update(const Wall& wall);

private:
    Real d;
    Vec3 mNormal;
    bool receiverValid;
    std::vector<size_t> mWalls;
};
```

---

## Wall

### `#!cpp Wall(const Vertices& vData, size_t materialId)`
**Constructor.**  
Initializes a wall from triangle vertices and a material reference.

`vData`: Vertices of the wall (stored as a triangle).  
`materialId`: ID of the material used by this wall.

---

### `#!cpp ~Wall()`
**Destructor.**  
Cleans up the wall.

---

### `#!cpp inline size_t GetMaterialID() const`
Returns the ID of the material associated with this wall.

---

### `#!cpp void Update(const Vertices& vData)`
Updates the wall vertices and derived values (normal, area, and plane distance).

`vData`: The new vertices of the wall.

---

## Plane

### `#!cpp Plane()`
**Default constructor.**  
Initializes an empty plane.

---

### `#!cpp Plane(const size_t id, const Wall& wall)`
**Constructor.**  
Initializes a plane and adds the given wall.

`id`: ID of the wall to add.  
`wall`: The wall being added.

---

## Example Usage

```cpp
#include "Spatialiser/Wall.h"
using namespace RAC::Spatialiser;

Vertices v = { Vec3(0,0,0), Vec3(1,0,0), Vec3(0,1,0) };
size_t materialId = 0;

Wall wall(v, materialId);

// Update geometry
Vertices v2 = { Vec3(0,0,0), Vec3(2,0,0), Vec3(0,2,0) };
wall.Update(v2);
```
