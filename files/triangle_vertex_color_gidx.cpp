#include "gidx.h"

/**
 * Renders a single triangle with interpolated vertex colors.
 */
int main()
{
    if (!Engine::Graphics(Engine::Renderer::DX11, 1280, 720, "Vertex Color Triangle"))
        return 1;

    // Create the camera.
    LPENTITY camera = NULL_LPENTITY;
    Engine::CreateCamera(&camera, 60.0f, 0.1f, 100.0f);
    Engine::PositionEntity(camera, 0.0f, 0.0f, -2.0f);
    Engine::LookAt(camera, 0.0f, 0.0f, 0.0f);

    // Create the directional light.
    LPENTITY light = NULL_LPENTITY;
    Engine::CreateLight(&light, LightKind::Directional, 1.0f, 1.0f, 1.0f);
    Engine::RotateEntity(light, 0.0f, 0.0f, 0.0f);

    // Build the triangle mesh data.
    SubmeshData triangle;
    triangle.positions = {
        { -0.7f, -0.6f, 0.0f },
        {  0.0f,  0.7f, 0.0f },
        {  0.7f, -0.6f, 0.0f },
    };
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
        { 1.0f, 0.0f, 0.0f, 1.0f }, // Left
        { 0.0f, 1.0f, 0.0f, 1.0f }, // Top
        { 0.0f, 0.0f, 1.0f, 1.0f }, // Right
    };
    triangle.indices = { 0, 1, 2 };

    if (!triangle.Validate())
        return 2;

    // Upload the mesh.
    LPMESH triangleMesh = Engine::UploadSubmesh(std::move(triangle), "VertexColorTriangle");
    if (triangleMesh.value == 0)
        return 3;

    // Create the render entity.
    LPENTITY triangleEntity = NULL_LPENTITY;
    Engine::CreateMesh(&triangleEntity, triangleMesh, "Triangle");

    // Create and assign a white material.
    LPMATERIAL material = NULL_MATERIAL;
    Engine::CreateMaterial(&material, 1.0f, 1.0f, 1.0f, 1.0f);
    Engine::AssignMaterial(triangleEntity, material);

    // Rotate the triangle every frame.
    Engine::OnUpdate([&](float dt)
        {
            Engine::TurnEntity(triangleEntity, 0.0f, 25.0f * dt, 0.0f);
        });

    Engine::Run();
    return 0;
}