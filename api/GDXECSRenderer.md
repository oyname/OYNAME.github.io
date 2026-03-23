# GDXECSRenderer

Zentraler Renderer und Haupt-API für Ressourcenverwaltung, Shader, Meshes, Materialien und Frame-Rendering.

## Deklaration

```cpp
class GDXECSRenderer
{
public:
    bool Init(IGDXRenderBackend* backend);
    void BeginFrame();
    void EndFrame();
    void Render();

    // Ressourcen
    MeshHandle     CreateMesh(const MeshDesc& desc);
    MaterialHandle CreateMaterial(const MaterialDesc& desc);
    TextureHandle  CreateTexture(const TextureDesc& desc);

    // ECS Zugriff
    Registry& GetRegistry();

    // Shader
    bool LoadShader(const std::string& path, ShaderStage stage);
};
```

## Methoden

### `Init()`

```cpp
bool Init(IGDXRenderBackend* backend);
```

Initialisiert den Renderer mit dem übergebenen Backend. Muss vor allen anderen Methoden aufgerufen werden.

---

### `BeginFrame()` / `EndFrame()`

```cpp
void BeginFrame();
void EndFrame();
```

Klammern einen Render-Pass. Zwischen diesen Aufrufen werden Draw-Calls platziert.

---

### `CreateMesh()`

```cpp
MeshHandle CreateMesh(const MeshDesc& desc);
```

Erstellt ein Mesh aus Vertex- und Indexdaten. Gibt ein Handle zurück, über das das Mesh referenziert wird.

---

### `GetRegistry()`

```cpp
Registry& GetRegistry();
```

Gibt die zentrale ECS-Registry zurück für Entitäten und Komponenten.

## Typischer Frame-Ablauf

```cpp
renderer->BeginFrame();

// Alle ECS-Entities mit RenderComponent werden automatisch gerendert
renderer->Render();

renderer->EndFrame();
```

## Screenshot

![Renderer Output](../assets/trinagle.png)

*Beispiel: Ein einfaches Triangle-Mesh, gerendert mit dem DX11-Backend.*

## Siehe auch

- [IGDXRenderBackend](IGDXRenderBackend)
- [Registry](Registry)
- [MeshHandle](MeshHandle)
