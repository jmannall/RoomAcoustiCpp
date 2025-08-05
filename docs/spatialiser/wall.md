Implements the `Wall` and `Plane` classes for geometric room modeling, including wall/plane intersection, reflection, and absorption properties.

- **Namespace:** `RAC::Spatialiser`
- **Header:** `Spatialiser/Wall.h`
- **Source:** `Spatialiser/Wall.cpp`
- **Dependencies:** `Common/Types.h`, `Common/Vec3.h`, `Common/Coefficients.h`, `Spatialiser/Edge.h`, `Spatialiser/Types.h`

---

## Class Definition

```cpp
class Wall
{
public:
    Wall();
    Wall(const Vertices& vData, const Absorption<>& absorption);
    ~Wall();

    inline void AddEdge(const size_t id);
    inline void RemoveEdge(const size_t id);
    inline bool EmptyEdges() const;
    inline Vec3 GetNormal() const;
    inline Real GetD() const;
    inline Vertices GetVertices() const;
    inline bool VertexMatch(const Vec3& x) const;
    inline Absorption<> GetAbsorption() const;
    inline Real GetArea() const;
    inline std::vector<size_t> GetEdges() const;
    inline size_t GetPlaneID() const;
    inline void SetPlaneID(const size_t id);
    inline Real PointWallPosition(const Vec3& point) const;
    bool LineWallIntersection(const Vec3& start, const Vec3& end, Vec3& intersection) const;
    inline bool LineWallObstruction(const Vec3& start, const Vec3& end) const;
    void Update(const Vertices& vData);
    inline void Update(const Absorption<>& absorption);

private:
    inline void CalculateArea();

    Vertices mVertices;
    Vec3 mNormal;
    Real d;
    Absorption<> mAbsorption;
    size_t mPlaneId;
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
    inline std::vector<size_t> GetWalls() const;
    inline bool GetReceiverValid() const;
    inline void SetReceiverValid(const Vec3& listenerPosition);
    bool IsCoplanar(const Wall& wall) const;
    Real PointPlanePosition(const Vec3& point) const;
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

## Public Methods

### `#!cpp Wall()`
**Default constructor.**  
Initializes an empty wall.

---

### `#!cpp Wall(const Vertices& vData, const Absorption<>& absorption)`
**Constructor.**  
Initializes a wall with vertices and absorption.
- `vData`: Vertices of the wall.
- `absorption`: Material absorption.

---

### `#!cpp ~Wall()`
**Destructor.**  
Cleans up the wall.

---

### `#!cpp inline void AddEdge(const size_t id)`
Adds the ID of a connecting edge.

---

### `#!cpp inline void RemoveEdge(const size_t id)`
Removes the ID of a connecting edge.

---

### `#!cpp inline bool EmptyEdges() const`
Returns true if the wall has unclaimed edges.

---

### `#!cpp inline Vec3 GetNormal() const`
Returns the wall normal.

---

### `#!cpp inline Real GetD() const`
Returns the wall's distance from the origin along the normal.

---

### `#!cpp inline Vertices GetVertices() const`
Returns the wall vertices.

---

### `#!cpp inline bool VertexMatch(const Vec3& x) const`
Returns true if the wall contains the given vertex.

---

### `#!cpp inline Absorption<> GetAbsorption() const`
Returns the wall's absorption.

---

### `#!cpp inline Real GetArea() const`
Returns the wall area.

---

### `#!cpp inline std::vector<size_t> GetEdges() const`
Returns the IDs of connected edges.

---

### `#!cpp inline size_t GetPlaneID() const`
Returns the plane ID.

---

### `#!cpp inline void SetPlaneID(const size_t id)`
Sets the plane ID.

---

### `#!cpp inline Real PointWallPosition(const Vec3& point) const`
Returns the signed distance from a point to the wall.

---

### `#!cpp bool LineWallIntersection(const Vec3& start, const Vec3& end, Vec3& intersection) const`
Checks if a line intersects the wall and stores the intersection point.
- `start`: Line start.
- `end`: Line end.
- `intersection`: Output intersection point.
- **Returns:** True if intersection occurs.

---

### `#!cpp inline bool LineWallObstruction(const Vec3& start, const Vec3& end) const`
Returns true if the wall obstructs a line.

---

### `#!cpp void Update(const Vertices& vData)`
Updates the wall's geometry and area.

---

### `#!cpp inline void Update(const Absorption<>& absorption)`
Updates the wall's absorption.

---

### `#!cpp Plane()`
**Default constructor.**  
Initializes an empty plane.

---

### `#!cpp Plane(const size_t id, const Wall& wall)`
**Constructor.**  
Initializes a plane with a wall.

---

### `#!cpp ~Plane()`
**Destructor.**  
Cleans up the plane.

---

### `#!cpp inline void AddWall(const size_t id)`
Adds a wall ID to the plane.

---

### `#!cpp inline bool RemoveWall(const size_t id)`
Removes a wall ID from the plane.

---

### `#!cpp inline Vec3 GetNormal() const`
Returns the plane normal.

---

### `#!cpp inline Real GetD() const`
Returns the plane's distance from the origin.

---

### `#!cpp inline std::vector<size_t> GetWalls() const`
Returns the wall IDs in the plane.

---

### `#!cpp inline bool GetReceiverValid() const`
Returns true if the listener is in front of the plane.

---

### `#!cpp inline void SetReceiverValid(const Vec3& listenerPosition)`
Sets whether the listener is in front of the plane.

---

### `#!cpp bool IsCoplanar(const Wall& wall) const`
Returns true if the wall is coplanar with the plane.

---

### `#!cpp Real PointPlanePosition(const Vec3& point) const`
Returns the signed distance from a point to the plane.

---

### `#!cpp bool LinePlaneObstruction(const Vec3& start, const Vec3& end) const`
Returns true if the plane obstructs a line.

---

### `#!cpp bool LinePlaneIntersection(const Vec3& start, const Vec3& end) const`
Returns true if a line intersects the plane.

---

### `#!cpp bool ReflectPointInPlane(const Vec3& point) const`
Returns true if the point is in front of the plane.

---

### `#!cpp bool ReflectPointInPlane(Vec3& dest, const Vec3& point) const`
Reflects a point in the plane and stores the result.

---

### `#!cpp void ReflectPointInPlaneNoCheck(Vec3& point) const`
Reflects a point in the plane without checking.

---

### `#!cpp void ReflectNormalInPlane(Vec3& normal) const`
Reflects a normal in the plane.

---

### `#!cpp bool EdgePlanePosition(const Edge& edge) const`
Returns true if an edge is in front of the plane.

---

### `#!cpp inline void Update(const Wall& wall)`
Updates the plane's normal and distance from a wall.

---

## Utility Functions

### `#!cpp std::pair<bool, Vec3> IntersectTriangle(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& origin, const Vec3& dir, const bool returnIntersection)`
Checks if a line intersects a triangle and optionally returns the intersection point.

---

## Internal Data Members

- `#!cpp Vertices mVertices`: Wall vertices.
- `#!cpp Vec3 mNormal`: Wall/plane normal.
- `#!cpp Real d`: Distance from origin.
- `#!cpp Absorption<> mAbsorption`: Wall absorption.
- `#!cpp size_t mPlaneId`: Plane ID.
- `#!cpp std::vector<size_t> mEdges`: Connected edge IDs.
- `#!cpp std::vector<size_t> mWalls`: Connected wall IDs (Plane).
- `#!cpp bool receiverValid`: True if listener is in front of the plane.

---

## Implementation Notes

- Walls are always triangles.
- Area is computed as half the cross product of two triangle edges.
- Includes geometric utilities for intersection and reflection.

## Example Usage

```cpp
#include "Spatialiser/Wall.h"
using namespace RAC::Spatialiser;

Vertices verts = { Vec3(0,0,0), Vec3(1,0,0), Vec3(0,1,0) };
Absorption<> abs(3);
Wall wall(verts, abs);

Vec3 intersection;
bool hit = wall.LineWallIntersection(Vec3(0,0,1), Vec3(0,0,-1), intersection);
```
