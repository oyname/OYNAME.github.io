# Bounds

`engine::BoundsComponent`

Axis-aligned bounding boxes in both local and world space. Used by the renderer for frustum culling and by physics queries for broadphase tests.

---

## BoundsComponent

```cpp
struct BoundsComponent {
    Vec3  centerLocal  { 0.f, 0.f, 0.f };
    Vec3  extentsLocal { 1.f, 1.f, 1.f };
    Vec3  centerWorld  { 0.f, 0.f, 0.f };
    Vec3  extentsWorld { 1.f, 1.f, 1.f };
    float boundingSphere          = 1.f;
    uint32_t lastTransformVersion = 0u;
    bool  localDirty              = true;
};
```

`centerLocal` and `extentsLocal` describe the AABB in object space. `centerWorld` and `extentsWorld` are computed from those values and the entity's `WorldTransformComponent`. `boundingSphere` is the radius of the bounding sphere in world space.

---

## BoundsSystem

`engine::BoundsSystem` — `scene/BoundsSystem.hpp`

Updates `centerWorld`, `extentsWorld`, and `boundingSphere` for every entity that has `MeshComponent`, `WorldTransformComponent`, and `BoundsComponent`.

```cpp
BoundsSystem bounds;
bounds.Update(world, assetRegistry);
```

Must be called after `TransformSystem::Update()` and after GPU mesh data is available, so that the local AABB from mesh vertices can be read from the registry.

---

## Update order

```cpp
transformSystem.Update(world);
boundsSystem.Update(world, registry);  // reads WorldTransformComponent
ExtractRenderData(world, renderQueue); // reads BoundsComponent.centerWorld
```

---

## Single-entity update

```cpp
BoundsSystem::ComputeBoundsForEntity(world, entity, registry);
```

Useful after a `SetParent` call or when only a subset of entities need their bounds refreshed outside the full system pass.

---

## Reading bounds in game code

```cpp
const auto* b = world.Get<BoundsComponent>(entity);
if (b) {
    Vec3 min = b->centerWorld - b->extentsWorld;
    Vec3 max = b->centerWorld + b->extentsWorld;
    float radius = b->boundingSphere;
}
```

---

## Notes

- Local extents default to `(1, 1, 1)`. Set them manually if no mesh is attached or before the asset is loaded.
- `BoundsSystem` uses a conservative OBB-to-AABB approximation when transforming. The world AABB may be larger than the tightest fit when the entity is rotated.
- Entities without a `BoundsComponent` are not culled by the renderer — they are always submitted for drawing.
