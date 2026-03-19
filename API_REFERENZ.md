# OYNAME2 ENGINE API REFERENCE

This reference describes how to use the Engine API.

The focus is on the actual engine path:

- Engine initialization
- Window and frame loop
- Event system and ESC behavior
- ECS usage
- Device enumeration under DX11
- Renderer and resource API
- Minimal startup example
- First triangle

Classification note:
The engine can be used in two ways. The primary path goes through the actual engine structure with direct access to the window, backend, renderer, ECS, and resources. In addition, `gidx.h` provides a simplified layer that wraps many standard steps. This reference describes the direct path first.

---

## 1. Architecture Overview

The engine is split into several clearly separated layers:

### `GDXEngine`
Top-level runtime for:
- window loop
- event processing
- delta time
- frame execution

### `GDXECSRenderer`
Central runtime API for:
- renderer initialization
- resource creation
- ECS registry
- tick callback
- frame execution
- render targets and post-processing

### `IGDXRenderBackend`
Abstract backend interface.

### `GDXDX11RenderBackend`
Concrete DX11 implementation.

### `Registry`
ECS storage for entities and components.

### `ResourceStore<T, Tag>`
Handle-based management for:
- meshes
- materials
- shaders
- textures
- render targets
- post-process resources

In practice, DX11 is the real main path in the current snapshot.

---

## 2. Initializing the Engine

### 2.1 `GDXEngine`

```cpp
class GDXEngine
{
public:
    GDXEngine(std::unique_ptr<IGDXWindow> window,
              std::unique_ptr<IGDXRenderer> renderer,
              GDXEventQueue& events);

    bool Initialize();
    void Run();
    bool Step();

    float GetDeltaTime() const;
    float GetTotalTime() const;

    using EventFn = std::function<void(const Event&)>;
    void SetEventCallback(EventFn fn);

    void Shutdown();
};
```

### Purpose

`GDXEngine` owns the window and renderer and controls the runtime loop.

### Important Methods

#### `bool Initialize()`
Initializes the engine.

Typical behavior:
- validates window and renderer
- calls `renderer->Initialize()`
- sets the initial viewport size via `renderer->Resize(...)`
- starts the frame timer

#### `void Run()`
Main application loop. Internally calls `Step()` repeatedly.

#### `bool Step()`
Executes exactly one frame.

Typical frame flow:
1. reset input state
2. poll window events
3. compute delta time
4. process events
5. skip rendering if the window is minimized
6. otherwise:
   - `BeginFrame()`
   - `Tick(dt)`
   - `EndFrame()`

Return value:
- `true` = keep running
- `false` = exit

#### `float GetDeltaTime() const`
Delta time of the last frame in seconds.

#### `float GetTotalTime() const`
Total runtime since startup.

#### `void Shutdown()`
Shuts the engine down cleanly. According to the header, this is idempotent, so calling it multiple times is safe.

---

## 3. Event System and ESC Behavior

### 3.1 Event Types

```cpp
enum class Key
{
    Unknown,
    Escape, Space,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Left, Right, Up, Down
};

struct QuitEvent {};
struct WindowResizedEvent { int width; int height; };
struct KeyPressedEvent { Key key; bool repeat; };
struct KeyReleasedEvent { Key key; };

using Event = std::variant<QuitEvent, WindowResizedEvent, KeyPressedEvent, KeyReleasedEvent>;
```

### 3.2 `GDXEventQueue`

The event queue is written by platform code and read by the engine once per frame.

Purpose:
- collect events
- process them consistently within the frame

### 3.3 ESC Behavior

ESC is already handled by the engine itself. That matters: you do not necessarily need your own game-side code for it.

In practice, this means:
- `Escape` marks the application to exit
- additional custom event code can still be registered

### 3.4 Custom Event Callback

```cpp
engine.SetEventCallback([&](const Event& e)
{
    std::visit([&](auto&& ev)
    {
        using T = std::decay_t<decltype(ev)>;

        if constexpr (std::is_same_v<T, KeyPressedEvent>)
        {
            if (ev.key == Key::Space)
            {
                // Custom reaction to Space
            }
        }
    }, e);
});
```

---

## 4. Input API

### `GDXInput`

```cpp
class GDXInput
{
public:
    static void BeginFrame();
    static void OnEvent(const Event& e);

    static bool KeyDown(Key key);
    static bool KeyHit(Key key);
    static bool KeyReleased(Key key);
};
```

### Meaning

#### `KeyDown(key)`
The key is currently being held.

#### `KeyHit(key)`
The key was pressed in this frame.

#### `KeyReleased(key)`
The key was released in this frame.

### Example

```cpp
if (GDXInput::KeyDown(Key::Left))
{
    // continuous movement to the left
}

if (GDXInput::KeyHit(Key::Space))
{
    // one-shot action on press
}
```

---

## 5. DX11 Device Enumeration and Context Creation

### `GDXWin32DX11ContextFactory`

```cpp
class GDXWin32DX11ContextFactory
{
public:
    static std::vector<GDXDXGIAdapterInfo> EnumerateAdapters();

    static unsigned int FindBestAdapter(
        const std::vector<GDXDXGIAdapterInfo>& adapters);

    std::unique_ptr<IGDXDXGIContext> Create(
        IGDXWin32NativeAccess& nativeAccess,
        unsigned int adapterIndex) const;
};
```

### Purpose

This factory creates a DX11 context for a Win32 window.

### `EnumerateAdapters()`
Reads available hardware adapters without permanently creating a device.

Use cases:
- display a GPU list
- let the user choose
- automatically select the best GPU

### `FindBestAdapter(...)`
Determines the adapter with the highest feature level.

This is the fastest default path when no manual GPU selection is needed.

### `Create(...)`
Creates a DX11 context for the Win32 window and the selected adapter.

Return value:
- valid `IGDXDXGIContext` on success
- `nullptr` if creation fails

### Example: enumerate adapters

```cpp
auto adapters = GDXWin32DX11ContextFactory::EnumerateAdapters();
for (size_t i = 0; i < adapters.size(); ++i)
{
    // print name, vendor, feature level, etc.
}
```

### Example: choose the best adapter

```cpp
auto adapters = GDXWin32DX11ContextFactory::EnumerateAdapters();
unsigned int bestAdapter = GDXWin32DX11ContextFactory::FindBestAdapter(adapters);
```

---

## 6. Renderer API

### 6.1 `IGDXRenderer`

```cpp
class IGDXRenderer
{
public:
    virtual ~IGDXRenderer() = default;

    virtual bool Initialize() = 0;
    virtual void BeginFrame() = 0;
    virtual void Tick(float dt) = 0;
    virtual void EndFrame() = 0;
    virtual void Resize(int w, int h) = 0;
    virtual void Shutdown() = 0;
};
```

This is the abstract base. The actual runtime work here goes through `GDXECSRenderer`.

### 6.2 `GDXECSRenderer`

```cpp
class GDXECSRenderer final : public IGDXRenderer
{
public:
    explicit GDXECSRenderer(std::unique_ptr<IGDXRenderBackend> backend);
    ~GDXECSRenderer() override;

    bool Initialize() override;
    void BeginFrame() override;
    void EndFrame() override;
    void Resize(int w, int h) override;
    void Shutdown() override;

    using TickFn = std::function<void(float)>;
    void SetTickCallback(TickFn fn);
    void Tick(float dt);

    Registry& GetRegistry();

    ShaderHandle   CreateShader(const std::wstring& vsFile,
                                const std::wstring& psFile,
                                uint32_t vertexFlags = GDX_VERTEX_DEFAULT);

    ShaderHandle   CreateShader(const std::wstring& vsFile,
                                const std::wstring& psFile,
                                uint32_t vertexFlags,
                                const GDXShaderLayout& layout);

    TextureHandle  LoadTexture(const std::wstring& filePath, bool isSRGB = true);
    TextureHandle  CreateTexture(const ImageBuffer& image,
                                 const std::wstring& debugName,
                                 bool isSRGB = true);

    MeshHandle     UploadMesh(MeshAssetResource mesh);
    MaterialHandle CreateMaterial(MaterialResource mat);

    ShaderHandle   GetDefaultShader() const;
    void SetShadowMapSize(uint32_t size);
    void SetSceneAmbient(float r, float g, float b);

    ResourceStore<MeshAssetResource, MeshTag>& GetMeshStore();
    ResourceStore<MaterialResource, MaterialTag>& GetMatStore();
    ResourceStore<GDXShaderResource, ShaderTag>& GetShaderStore();
    ResourceStore<GDXTextureResource, TextureTag>& GetTextureStore();

    RenderTargetHandle CreateRenderTarget(uint32_t w, uint32_t h,
                                          const std::wstring& name,
                                          GDXTextureFormat colorFormat = GDXTextureFormat::RGBA8_UNORM);

    TextureHandle GetRenderTargetTexture(RenderTargetHandle h);

    PostProcessHandle CreatePostProcessPass(const PostProcessPassDesc& desc);
    bool SetPostProcessConstants(PostProcessHandle h, const void* data, uint32_t size);
    bool SetPostProcessEnabled(PostProcessHandle h, bool enabled);
    void ClearPostProcessPasses();

    void SetClearColor(float r, float g, float b, float a = 1.0f);
};
```

### Core Responsibilities

`GDXECSRenderer` is the actual high-level runtime for:
- ECS access
- resource creation
- render-frame execution
- shader/texture/mesh/material management
- render-to-texture and post-processing

---

## 7. Using ECS

### 7.1 `Registry`

The `Registry` is the central ECS object.

Important operations:

```cpp
EntityID CreateEntity();
void DestroyEntity(EntityID id);
bool IsAlive(EntityID id) const;
size_t EntityCount() const;

template<typename T, typename... Args>
T& Add(EntityID id, Args&&... args);

template<typename T>
T* Get(EntityID id);

template<typename T>
bool Has(EntityID id) const;

template<typename T>
void Remove(EntityID id);

template<typename First, typename... Rest, typename Func>
void View(Func&& func);
```

### Basic Pattern

```cpp
Registry& registry = renderer.GetRegistry();

EntityID entity = registry.CreateEntity();
registry.Add<TagComponent>(entity, TagComponent{"Triangle"});
```

### Reading Components

```cpp
auto* transform = registry.Get<TransformComponent>(entity);
if (transform)
{
    transform->localPosition = {0.0f, 0.0f, 5.0f};
    transform->dirty = true;
}
```

### Iterating Over Entities

```cpp
registry.View<TransformComponent, RenderableComponent>(
    [&](EntityID id, TransformComponent& transform, RenderableComponent& renderable)
    {
        // all entities with these components
    });
```

### Important

The registry is handle- and component-based. Render resources themselves are not stored directly in components but referenced through handles.

---

## 8. Important Components

### `TagComponent`
Human-readable name of an entity.

```cpp
struct TagComponent
{
    std::string name;
};
```

### `TransformComponent`
Local transform data.

```cpp
struct TransformComponent
{
    GIDX::Float3 localPosition;
    GIDX::Float4 localRotation;
    GIDX::Float3 localScale;

    bool dirty;
    uint32_t localVersion;
    uint32_t worldVersion;

    void SetEulerDeg(float pitchDeg, float yawDeg, float rollDeg);
};
```

Important:
- contains local data only
- the world matrix is stored separately in `WorldTransformComponent`

### `WorldTransformComponent`
Computed world matrix and inverse. Written by the transform system.

### `RenderableComponent`
Links an entity to a renderable resource.

```cpp
struct RenderableComponent
{
    MeshHandle mesh;
    MaterialHandle material;
    uint32_t submeshIndex = 0u;
    bool enabled = true;

    bool dirty = true;
    uint32_t stateVersion = 1u;
};
```

### `VisibilityComponent`
Visibility, activity, layer, and shadow behavior.

```cpp
struct VisibilityComponent
{
    bool visible = true;
    bool active = true;
    uint32_t layerMask = 0x00000001u;
    bool castShadows = true;
    bool receiveShadows = true;

    bool dirty = true;
    uint32_t stateVersion = 1u;
};
```

### `RenderBoundsComponent`
Bounds used for culling.

### `CameraComponent`
Projection parameters.

```cpp
struct CameraComponent
{
    float fovDeg = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    float aspectRatio = 16.0f / 9.0f;

    bool  isOrtho = false;
    float orthoWidth = 10.0f;
    float orthoHeight = 10.0f;

    uint32_t cullMask = 0xFFFFFFFFu;
};
```

### `ActiveCameraTag`
Marks the active camera.

### `RenderTargetCameraComponent`
Camera that renders into an offscreen target.

### `LightComponent`
Lighting data for directional, point, and spot lights.

---

## 9. Creating Resources

### 9.1 Shaders

```cpp
ShaderHandle shader = renderer.CreateShader(
    L"shader/ECSVertexShader.hlsl",
    L"shader/ECSPixelShader.hlsl",
    GDX_VERTEX_DEFAULT);
```

Or with an explicit layout:

```cpp
ShaderHandle shader = renderer.CreateShader(vsFile, psFile, vertexFlags, customLayout);
```

### 9.2 Textures

From file:

```cpp
TextureHandle tex = renderer.LoadTexture(L"assets/albedo.png", true);
```

From CPU image data:

```cpp
TextureHandle tex = renderer.CreateTexture(imageBuffer, L"GeneratedTexture", true);
```

### 9.3 Meshes

```cpp
MeshAssetResource mesh = /* build mesh data */;
MeshHandle meshHandle = renderer.UploadMesh(std::move(mesh));
```

### 9.4 Materials

```cpp
MaterialResource mat{};
MaterialHandle matHandle = renderer.CreateMaterial(std::move(mat));
```

---

## 10. Render Targets and Post-Processing

### Create a render target

```cpp
RenderTargetHandle rt = renderer.CreateRenderTarget(
    1024,
    1024,
    L"OffscreenColor",
    GDXTextureFormat::RGBA8_UNORM);
```

### Get the associated texture

```cpp
TextureHandle rtTexture = renderer.GetRenderTargetTexture(rt);
```

### Create a post-process pass

```cpp
PostProcessPassDesc desc{};
PostProcessHandle pp = renderer.CreatePostProcessPass(desc);
```

### Set constants

```cpp
renderer.SetPostProcessConstants(pp, &myData, sizeof(myData));
```

### Enable/disable

```cpp
renderer.SetPostProcessEnabled(pp, true);
```

### Remove all passes

```cpp
renderer.ClearPostProcessPasses();
```

---

## 11. Configuring the Renderer

### Clear Color

```cpp
renderer.SetClearColor(0.05f, 0.05f, 0.12f, 1.0f);
```

### Scene Ambient

```cpp
renderer.SetSceneAmbient(0.08f, 0.08f, 0.12f);
```

### Shadow Map Size

```cpp
renderer.SetShadowMapSize(2048);
```

---

## 12. Using the Tick Callback

The renderer can execute game or demo update code once per frame.

```cpp
renderer.SetTickCallback([&](float dt)
{
    // game update, ECS changes, animation, camera, etc.
});
```

The engine then calls `renderer.Tick(dt)` each frame.

---

## 13. Minimal Engine Initialization

This example shows the direct path through window, DX11 context, backend, renderer, and engine.

```cpp
#include "GDXEngine.h"
#include "GDXECSRenderer.h"
#include "GDXWin32Window.h"
#include "GDXWin32DX11ContextFactory.h"
#include "GDXDX11RenderBackend.h"
#include "WindowDesc.h"

int main()
{
    GDXEventQueue events;

    WindowDesc desc{};
    desc.width = 1280;
    desc.height = 720;
    desc.title = "GIDX Demo";

    auto window = std::make_unique<GDXWin32Window>(desc, events);

    auto adapters = GDXWin32DX11ContextFactory::EnumerateAdapters();
    if (adapters.empty())
        return 1;

    unsigned int bestAdapter = GDXWin32DX11ContextFactory::FindBestAdapter(adapters);

    GDXWin32DX11ContextFactory factory;
    auto* nativeAccess = dynamic_cast<IGDXWin32NativeAccess*>(window.get());
    if (!nativeAccess)
        return 1;

    auto dxContext = factory.Create(*nativeAccess, bestAdapter);
    if (!dxContext)
        return 1;

    auto backend = std::make_unique<GDXDX11RenderBackend>(std::move(dxContext));
    auto renderer = std::make_unique<GDXECSRenderer>(std::move(backend));

    GDXEngine engine(std::move(window), std::move(renderer), events);

    if (!engine.Initialize())
        return 1;

    engine.Run();
    engine.Shutdown();
    return 0;
}
```

Note:
The factory comment in the code explicitly states that the DX11 factory is designed to work directly with `IGDXWin32NativeAccess` and does not want a `dynamic_cast` dependency in the actual factory design. The cast above is therefore just a pragmatic example at the application level.

---

## 14. Rendering the First Triangle

The honest answer is:
In this engine, a raw triangle does not come from a simple immediate-mode draw call, but through the normal ECS/mesh/material path. That means at minimum you need to:

1. initialize the engine
2. create and upload a mesh
3. create a material
4. create a camera entity
5. create a renderable entity

### 14.1 Create the active camera

```cpp
Registry& registry = renderer.GetRegistry();

EntityID camera = registry.CreateEntity();
registry.Add<TagComponent>(camera, TagComponent{"MainCamera"});
registry.Add<TransformComponent>(camera, TransformComponent{0.0f, 0.0f, -3.0f});
registry.Add<WorldTransformComponent>(camera, WorldTransformComponent{});

CameraComponent cam{};
cam.fovDeg = 60.0f;
cam.nearPlane = 0.1f;
cam.farPlane = 100.0f;
cam.aspectRatio = 1280.0f / 720.0f;
registry.Add<CameraComponent>(camera, cam);
registry.Add<ActiveCameraTag>(camera, ActiveCameraTag{});
```

### 14.2 Create the triangle mesh

For this, you need a `MeshAssetResource` with exactly one submesh. The exact fields depend on `MeshAssetResource` and `SubmeshData`. The general idea is:

```cpp
MeshAssetResource mesh{};

// set vertex data for 3 points
// index data: 0, 1, 2
// choose a matching vertex layout/flag set
// create one submesh

MeshHandle meshHandle = renderer.UploadMesh(std::move(mesh));
```

### 14.3 Create the material

```cpp
MaterialResource material{};
// set shader/texture/material parameters

MaterialHandle materialHandle = renderer.CreateMaterial(std::move(material));
```

### 14.4 Create the renderable entity

```cpp
EntityID triangle = registry.CreateEntity();
registry.Add<TagComponent>(triangle, TagComponent{"Triangle"});
registry.Add<TransformComponent>(triangle, TransformComponent{0.0f, 0.0f, 0.0f});
registry.Add<WorldTransformComponent>(triangle, WorldTransformComponent{});
registry.Add<RenderableComponent>(triangle, RenderableComponent{meshHandle, materialHandle, 0u});
registry.Add<VisibilityComponent>(triangle, VisibilityComponent{});
registry.Add<RenderBoundsComponent>(triangle,
    RenderBoundsComponent::MakeSphere({0.0f, 0.0f, 0.0f}, 1.0f));
```

### 14.5 Continuous update code

```cpp
renderer.SetTickCallback([&](float dt)
{
    auto* t = registry.Get<TransformComponent>(triangle);
    if (t)
    {
        t->SetEulerDeg(0.0f, engine.GetTotalTime() * 50.0f, 0.0f);
        t->dirty = true;
    }
});
```

### Important

The critical part is not engine initialization, but the exact construction of `MeshAssetResource` and `MaterialResource`. The basic principle is clear, but for a fully compilable triangle sample you need to populate both types according to their exact fields.

---

## 15. Typical Minimal ECS Render Path

For an entity to actually be visible, you usually need:

- `TransformComponent`
- `WorldTransformComponent`
- `RenderableComponent`
- `VisibilityComponent`
- `RenderBoundsComponent`

For an active camera:

- `TransformComponent`
- `WorldTransformComponent`
- `CameraComponent`
- `ActiveCameraTag`

If this baseline is missing, nothing will render even if the mesh and material exist correctly.

---

## 16. What `gidx.h` Does Differently

In addition to the direct engine API, `gidx.h` provides a simplified access layer.

This is useful for:
- quick demos
- samples
- simple tools
- fast onboarding

It is not the primary low-/mid-level access path, but a convenience layer that wraps parts of initialization and standard usage.

If you actually want to understand or extend the engine structure, you should work with the direct path through `GDXEngine`, window, backend, and `GDXECSRenderer`.

---

## 17. Summary

Actual engine usage goes through:

1. create a Win32 window
2. enumerate DX11 adapters
3. create a DX11 context
4. create the backend
5. create `GDXECSRenderer`
6. create and initialize `GDXEngine`
7. create ECS entities, camera, meshes, and materials
8. run the frame loop with `Run()` or `Step()`

A first visible object does not come from a naked draw call. The engine is clearly built around ECS and resource flow. That is why a first triangle is a normal mesh/material/entity setup case, not a special path.
