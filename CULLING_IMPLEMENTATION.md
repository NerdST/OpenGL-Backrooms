# Frustum and Occlusion Culling Implementation

This document describes the implementation of frustum and occlusion culling systems for the OpenGL Backrooms maze renderer.

## Overview

Two culling systems have been implemented to optimize rendering performance:

1. **Frustum Culling** - Eliminates objects outside the camera's view frustum
2. **Occlusion Culling** - Eliminates objects hidden behind other geometry

## Files Added

### Headers
- `include/FrustumCuller.h` - Frustum culling class definition
- `include/OcclusionCuller.h` - Occlusion culling class definition

### Source Files
- `src/FrustumCuller.cpp` - Frustum culling implementation
- `src/OcclusionCuller.cpp` - Occlusion culling implementation

## Implementation Details

### Frustum Culling

**Class: `FrustumCuller`**

The frustum culler extracts the 6 frustum planes (left, right, top, bottom, near, far) from the view-projection matrix and tests objects against these planes.

**Key Features:**
- Plane extraction from view-projection matrix
- AABB (Axis-Aligned Bounding Box) testing against frustum
- Point and sphere testing capabilities
- Efficient positive vertex testing for AABB culling

**Data Structures:**
```cpp
struct Plane {
    glm::vec3 normal;
    float distance;
};

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};
```

**Main Methods:**
- `updateFrustum(viewProjectionMatrix)` - Extract frustum planes
- `isAABBVisible(aabb)` - Test if bounding box is visible
- `isPointVisible(point)` - Test if point is visible
- `isSphereVisible(center, radius)` - Test if sphere is visible

### Occlusion Culling

**Class: `OcclusionCuller`**

The occlusion culler uses heuristic-based testing to determine if objects are likely occluded by maze walls. This is a simplified implementation that uses ray-marching through the maze grid rather than complex GPU occlusion queries.

**Key Features:**
- Heuristic-based occlusion testing
- Line-of-sight checking through maze grid
- Visibility caching for performance
- Distance-based culling

**Main Methods:**
- `initialize()` - Set up the occlusion system
- `beginFrame()` - Start culling for current frame
- `shouldRenderCell(cellPos, worldPos, cameraPos, maze)` - Test if cell should be rendered
- `endFrame()` - Finish culling for current frame

**Heuristic Algorithm:**
1. Check distance from camera (skip if too far)
2. Check visibility cache for quick lookup
3. Perform ray-march test from camera to object
4. Cache results for future frames

## Integration in Main Renderer

### Initialization
```cpp
// In main()
occlusionCuller.initialize();
```

### Per-Frame Updates
```cpp
// In render loop
if (enableFrustumCulling) {
    frustumCuller.updateFrustum(projection * view);
}
if (enableOcclusionCulling) {
    occlusionCuller.beginFrame();
}
```

### Per-Object Testing
```cpp
// For each maze cell
bool shouldRender = true;

// Frustum culling
if (enableFrustumCulling) {
    AABB cellAABB(min, max);
    shouldRender = frustumCuller.isAABBVisible(cellAABB);
}

// Occlusion culling
if (enableOcclusionCulling && shouldRender) {
    shouldRender = occlusionCuller.shouldRenderCell(cellPos, worldPos, cameraPos, maze);
}

if (shouldRender) {
    // Render the cell
    cellsRendered++;
} else {
    cellsCulled++;
}
```

## UI Controls

The following controls have been added to the ImGui interface:

### Culling Statistics
- **Cells Rendered** - Number of cells drawn this frame
- **Cells Culled** - Number of cells skipped this frame
- **Culling Efficiency** - Percentage of cells culled
- **Active Occlusion Queries** - Number of ongoing occlusion tests
- **Occluded Cells** - Number of cells occluded by geometry

### Culling Options
- **Enable Frustum Culling** - Toggle frustum culling on/off
- **Enable Occlusion Culling** - Toggle occlusion culling on/off

## Performance Benefits

### Frustum Culling
- Eliminates rendering of cells outside the camera's field of view
- Particularly effective when looking down corridors or in specific directions
- Provides immediate performance boost with minimal overhead

### Occlusion Culling
- Reduces rendering of cells hidden behind walls
- Most effective in maze environments with many walls
- Uses distance-based optimization to focus on nearby objects

### Expected Performance Gains
- **Frustum Culling**: 30-50% reduction in rendered cells when looking down corridors
- **Occlusion Culling**: 20-40% additional reduction in complex maze areas
- **Combined**: Up to 70% reduction in rendered geometry in optimal scenarios

## Configuration

### Frustum Culling
- No configuration needed - automatically adapts to camera settings

### Occlusion Culling
- `OCCLUSION_DISTANCE = 15.0f` - Maximum distance for occlusion testing
- `MAX_QUERIES = 512` - Maximum number of cached visibility results
- `QUERY_FREQUENCY = 3` - Frames between occlusion tests for non-visible objects

## Technical Notes

### Frustum Plane Extraction
The implementation uses the standard method of extracting frustum planes from the view-projection matrix by combining specific rows and columns. This provides accurate plane equations for testing.

### AABB vs Frustum Testing
The frustum culling uses the "positive vertex" method, which tests only the vertex of the AABB that is farthest along the plane normal. If this vertex is outside the plane, the entire AABB is outside.

### Occlusion Heuristics
The occlusion system uses a simplified ray-marching approach that steps through the maze grid checking for wall intersections. This provides good results for grid-based environments without the complexity of GPU occlusion queries.

### Memory Management
Both systems use efficient data structures and avoid dynamic memory allocation during rendering. Visibility results are cached to prevent redundant calculations.

## Future Improvements

1. **GPU Occlusion Queries** - Implement hardware occlusion queries for more accurate results
2. **Hierarchical Culling** - Add spatial partitioning for larger scenes
3. **Temporal Coherence** - Improve frame-to-frame consistency in culling decisions
4. **Portal Culling** - Specialized culling for corridor-based environments
5. **Level-of-Detail** - Dynamic quality reduction for distant objects

## Usage

The culling systems are integrated seamlessly into the existing renderer. Users can toggle them on/off through the ImGui interface and observe the performance statistics in real-time. The systems automatically adapt to camera movement and provide optimal performance in typical maze navigation scenarios.