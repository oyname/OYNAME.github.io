#include "GIDXEngine.h"
#include "GDXEventQueue.h"
#include "WindowDesc.h"
#include "GDXWin32Window.h"
#include "GDXWin32DX11ContextFactory.h"
#include "GDXDX11RenderBackend.h"
#include "GDXECSRenderer.h"
#include "Components.h"
#include "MeshAssetResource.h"
#include "MaterialResource.h"
#include "Events.h"
#include "SubmeshData.h"

#include <memory>
#include <type_traits>
#include <variant>

/**
 * Creates a single triangle with per-vertex color data.
 * The vertex colors are interpolated across the triangle during rasterization.
 */
static SubmeshData MakeColoredTriangle()
{
    SubmeshData triangle;

    triangle.positions = {
        { -0.7f, -0.6f, 0.0f },
        {  0.0f,  0.7f, 0.0f },
        {  0.7f, -0.6f, 0.0f },
    };

    // Vertex normals used for lighting.
    // This example uses a normal of (0, 0, -1) for all three vertices.
    triangle.normals = {
        { 0.0f, 0.0f, -1.0f },
        { 0.0f, 0.0f, -1.0f },
        { 0.0f, 0.0f, -1.0f },
    };

    triangle.uv0 = {
        { 0.0f, 1.0f },
        { 0.5f, 0.0f },
        { 1.0f, 1.0f },
    };

    triangle.colors = {
        { 1.0f, 0.0f, 0.0f, 1.0f }, // Red
        { 0.0f, 1.0f, 0.0f, 1.0f }, // Green
        { 0.0f, 0.0f, 1.0f, 1.0f }, // Blue
    };

    triangle.indices = { 0, 1, 2 };
    return triangle;
}

class TriangleExample
{
public:
    /**
     * Initializes the example with a renderer reference.
     */
    explicit TriangleExample(GDXECSRenderer& renderer)
        : m_renderer(renderer)
    {
    }

    /**
     * Creates and uploads the triangle mesh, then builds the scene:
     * camera, directional light, triangle entity, and ambient light.
     */
    void Init()
    {
        Registry& reg = m_renderer.GetRegistry();

        MeshAssetResource meshAsset;
        meshAsset.debugName = "VertexColorTriangle";
        meshAsset.AddSubmesh(MakeColoredTriangle());
        m_triangleMesh = m_renderer.UploadMesh(std::move(meshAsset));

        // Use a white base material so the interpolated vertex colors stay visible.
        m_whiteMaterial = m_renderer.CreateMaterial(
            MaterialResource::FlatColor(1.0f, 1.0f, 1.0f, 1.0f));

        m_camera = reg.CreateEntity();
        reg.Add<TagComponent>(m_camera, "Camera");
        {
            TransformComponent tc;
            tc.localPosition = { 0.0f, 0.0f, -2.0f };
            reg.Add<TransformComponent>(m_camera, tc);
        }
        reg.Add<WorldTransformComponent>(m_camera);
        {
            CameraComponent cam;
            cam.aspectRatio = 1280.0f / 720.0f;
            cam.nearPlane = 0.1f;
            cam.farPlane = 100.0f;
            cam.fovDeg = 60.0f;
            reg.Add<CameraComponent>(m_camera, cam);
        }
        reg.Add<ActiveCameraTag>(m_camera);

        m_light = reg.CreateEntity();
        reg.Add<TagComponent>(m_light, "Sun");
        {
            LightComponent lc;
            lc.kind = LightKind::Directional;
            lc.diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
            lc.intensity = 1.0f;
            reg.Add<LightComponent>(m_light, lc);
        }
        {
            TransformComponent tc;
            tc.SetEulerDeg(0.0f, 0.0f, 0.0f);
            reg.Add<TransformComponent>(m_light, tc);
        }
        reg.Add<WorldTransformComponent>(m_light);

        m_triangle = reg.CreateEntity();
        reg.Add<TagComponent>(m_triangle, "Triangle");
        {
            TransformComponent tc;
            tc.localPosition = { 0.0f, 0.0f, 0.0f };
            reg.Add<TransformComponent>(m_triangle, tc);
        }
        reg.Add<WorldTransformComponent>(m_triangle);
        reg.Add<RenderableComponent>(m_triangle, m_triangleMesh, m_whiteMaterial, 0u);
        reg.Add<VisibilityComponent>(m_triangle);

        m_renderer.SetSceneAmbient(0.10f, 0.10f, 0.10f);
    }

    /**
     * Rotates the triangle continuously around the Y axis.
     */
    void Update(float dt)
    {
        m_yaw += 35.0f * dt;
        while (m_yaw >= 360.0f)
            m_yaw -= 360.0f;

        if (auto* tc = m_renderer.GetRegistry().Get<TransformComponent>(m_triangle))
            tc->SetEulerDeg(0.0f, m_yaw, 0.0f);
    }

    /**
     * Processes application input events.
     * Press Escape to stop the engine.
     */
    void OnEvent(const Event& e, GIDXEngine& engine)
    {
        std::visit([&](auto&& ev)
            {
                using T = std::decay_t<decltype(ev)>;
                if constexpr (std::is_same_v<T, KeyPressedEvent>)
                {
                    if (!ev.repeat && ev.key == Key::Escape)
                        engine.Shutdown();
                }
            }, e);
    }

private:
    GDXECSRenderer& m_renderer;
    EntityID m_camera = NULL_ENTITY;
    EntityID m_light = NULL_ENTITY;
    EntityID m_triangle = NULL_ENTITY;
    MeshHandle m_triangleMesh{};
    MaterialHandle m_whiteMaterial{};
    float m_yaw = 0.0f;
};

/**
 * Application entry point.
 * Creates the window, graphics context, renderer, engine, and example scene,
 * then starts the main loop.
 */
int main()
{
    GDXEventQueue events;

    WindowDesc desc;
    desc.width = 1280;
    desc.height = 720;
    desc.title = "GIDX - Colored Triangle";
    desc.resizable = true;

    auto windowOwned = std::make_unique<GDXWin32Window>(desc, events);
    if (!windowOwned->Create())
        return 1;

    auto adapters = GDXWin32DX11ContextFactory::EnumerateAdapters();
    if (adapters.empty())
        return 2;

    GDXWin32DX11ContextFactory dx11Factory;
    auto dxContext = dx11Factory.Create(
        *windowOwned,
        GDXWin32DX11ContextFactory::FindBestAdapter(adapters));
    if (!dxContext)
        return 3;

    auto backendOwned = std::make_unique<GDXDX11RenderBackend>(std::move(dxContext));
    auto rendererOwned = std::make_unique<GDXECSRenderer>(std::move(backendOwned));
    GDXECSRenderer* renderer = rendererOwned.get();
    renderer->SetClearColor(0.05f, 0.06f, 0.09f);

    GIDXEngine engine(std::move(windowOwned), std::move(rendererOwned), events);
    if (!engine.Initialize())
        return 4;

    TriangleExample app(*renderer);
    app.Init();

    renderer->SetTickCallback([&](float dt)
        {
            app.Update(dt);
        });

    engine.SetEventCallback([&](const Event& e)
        {
            app.OnEvent(e, engine);
        });

    engine.Run();
    engine.Shutdown();
    return 0;
}
