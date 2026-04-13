# Material

`engine::renderer::MaterialSystem` · `engine::renderer::MaterialDesc` · `engine::renderer::MaterialHandle`

Materials describe **what is rendered**, **how it is shaded**, and **which fixed pipeline state is required**. A material combines shader references, render-pass classification, vertex layout, render target formats, depth/blend/rasterizer state, semantic material inputs, and optional explicit shader parameters.

`MaterialSystem` stores and manages registered material descriptions. A material can be used directly or cloned as an instance. Instances inherit the base material setup and can override parameter values without changing the underlying pipeline as long as the pipeline-relevant state stays identical.

---

## What belongs to a material

A `MaterialDesc` can define:

- material identity: `name`
- render-pass usage: `passTag`
- shading model: `model`
- shaders: `vertexShader`, `fragmentShader`
- vertex input: `vertexLayout`
- output formats: `colorFormat`, `depthFormat`
- fixed pipeline state:
  - `rasterizer`
  - `blend`
  - `depthStencil`
  - `renderPolicy`
- explicit shader parameters: `params`
- semantic material inputs:
  - `semanticTextures[...]`
  - `semanticValues[...]`

The current system supports both:

- **explicit parameter-driven materials** for shaders that bind named parameters directly
- **engine-semantic materials** for standard material inputs such as base color, metallic, roughness, normal, emissive, and related factors

---

## Registering a material

```cpp
renderer::MaterialDesc desc{};
desc.name = "CubePBR";
desc.passTag = renderer::RenderPassTag::Opaque;
desc.model = renderer::MaterialModel::PBRMetalRough;
desc.vertexShader = vsHandle;
desc.fragmentShader = psHandle;
desc.vertexLayout = vLayout;
desc.colorFormat = renderer::Format::RGBA16_FLOAT;
desc.depthFormat = renderer::Format::D24_UNORM_S8_UINT;

desc.semanticTextures[static_cast<size_t>(renderer::MaterialSemantic::BaseColor)] = {
    .set = true,
    .texture = gpuTex
};

desc.semanticValues[static_cast<size_t>(renderer::MaterialSemantic::BaseColor)] = {
    .set = true,
    .data = { 1.f, 1.f, 1.f, 1.f }
};

desc.semanticValues[static_cast<size_t>(renderer::MaterialSemantic::Metallic)] = {
    .set = true,
    .data = { 0.0f }
};

desc.semanticValues[static_cast<size_t>(renderer::MaterialSemantic::Roughness)] = {
    .set = true,
    .data = { 0.5f }
};

desc.renderPolicy.cullMode = renderer::MaterialCullMode::None;
desc.renderPolicy.castShadows = true;
desc.renderPolicy.receiveShadows = true;

renderer::MaterialHandle mat = materials.RegisterMaterial(std::move(desc));
```

This creates a backend-neutral material description. The actual backend pipeline object is derived later from the material state and cached through the pipeline cache.

---

## Explicit parameter-based materials

Materials can still be authored with explicit named parameters when a shader expects direct bindings instead of engine semantics.

```cpp
renderer::MaterialParam albedoParam{};
albedoParam.name = "albedo";
albedoParam.type = renderer::MaterialParam::Type::Texture;
albedoParam.texture = gpuTex;

renderer::MaterialParam samplerParam{};
samplerParam.name = "sampler_albedo";
samplerParam.type = renderer::MaterialParam::Type::Sampler;
samplerParam.samplerIdx = 0u;

renderer::MaterialDesc desc{};
desc.name = "CubeUnlit";
desc.passTag = renderer::RenderPassTag::Opaque;
desc.vertexShader = vsHandle;
desc.fragmentShader = psHandle;
desc.vertexLayout = vLayout;
desc.colorFormat = renderer::Format::RGBA16_FLOAT;
desc.depthFormat = renderer::Format::D24_UNORM_S8_UINT;
desc.params.push_back(albedoParam);
desc.params.push_back(samplerParam);

renderer::MaterialHandle mat = materials.RegisterMaterial(std::move(desc));
```

This path is useful for custom shaders and non-standard bindings. It is also the safer option when a backend-specific semantic path is not yet fully implemented.

---

## Semantic material inputs

The semantic material path is the preferred way to describe standard surface data in a backend-neutral form.

Typical semantics include:

- `BaseColor`
- `Normal`
- `Metallic`
- `Roughness`
- `Emissive`
- related factor values such as color multipliers or scalar overrides

Each semantic slot can be provided either as:

- a texture via `semanticTextures[...]`
- a fallback value via `semanticValues[...]`

This allows flexible material authoring:

- base color only
- base color + normal
- base color + metallic/roughness
- full PBR
- unlit-style usage with only selected semantics

If a texture is missing, the runtime and shader path can fall back to the semantic value or backend-provided default textures, depending on the material model and shader implementation.

---

## Material model

`MaterialDesc::model` describes the intended shading model.

Common cases:

- `renderer::MaterialModel::PBRMetalRough`
- backend- or shader-specific simpler models for unlit or custom shading

The material model does **not** replace shader selection. Shaders still define the actual code path. The model tells the engine how to interpret semantic material inputs and how the material should be treated by runtime systems.

---

## Creating instances

Material instances are derived from a registered base material.

```cpp
renderer::MaterialHandle redInstance = materials.CreateInstance(baseMaterial, "RedInstance");
materials.SetVec4(redInstance, "baseColor", { 1.f, 0.f, 0.f, 1.f });
```

Instances reuse the same pipeline state as long as no pipeline-relevant fields differ. This means parameter variation does not automatically create a new PSO.

---

## Setting parameters

Named parameters can be updated through the material system.

```cpp
materials.SetFloat(handle, "roughness", 0.4f);
materials.SetVec4(handle, "baseColor", { 0.8f, 0.6f, 0.2f, 1.f });
materials.SetTexture(handle, "albedoMap", texHandle);
materials.MarkDirty(handle);
```

Current behavior:

- `SetFloat` and `SetVec4` update parameter data and mark the material dirty automatically
- `SetTexture` updates the texture binding, but an explicit `MarkDirty(handle)` may still be required depending on the path and backend

When in doubt, call `MarkDirty(handle)` after changing texture-bound parameters.

---

## Constant buffer data

For parameter-driven materials, the system can pack parameter values into constant buffer layout data.

```cpp
const std::vector<uint8_t>& cbData = materials.GetCBData(handle);
const CbLayout& cbLayout = materials.GetCBLayout(handle);

uint32_t offset = cbLayout.GetOffset("roughness"); // UINT32_MAX if not found
```

Packing follows the engine's constant-buffer layout rules, aligned to the runtime shader binding model used by the active backend.

---

## Render pass tags

| Tag | Usage |
|---|---|
| `RenderPassTag::Opaque` | Default forward pass for opaque geometry |
| `RenderPassTag::AlphaCutout` | Masked geometry with alpha test / cutout logic |
| `RenderPassTag::Transparent` | Transparent geometry, typically back-to-front |
| `RenderPassTag::Shadow` | Shadow map rendering pass |
| `RenderPassTag::UI` | Overlay/UI rendering |
| `RenderPassTag::Postprocess` | Full-screen and post-processing passes |

The render pass tag determines where and how the material is scheduled inside the frame.

---

## Render policy

`MaterialDesc::renderPolicy` contains engine-level rendering behavior that is not just raw backend state.

Typical fields include:

- `cullMode`
- `castShadows`
- `receiveShadows`

This separates higher-level engine policy from low-level rasterizer, blend, and depth state.

---

## Pipeline key and caching

The pipeline state derived from a material is converted into a cache key and looked up in the pipeline cache.

```cpp
PipelineKey key = materials.BuildPipelineKey(handle);
```

Two materials with identical pipeline-relevant state share the same pipeline key and therefore the same underlying PSO or backend pipeline object, regardless of differing parameter values.

Pipeline-relevant state generally includes:

- shaders
- vertex layout
- render target formats
- depth format
- rasterizer state
- blend state
- depth/stencil state
- pass classification where relevant

Parameter data and material instance values do not by themselves create a separate pipeline unless they imply a different pipeline state.

---

## Sort key

Draw calls are sorted before submission to reduce state changes and preserve correct rendering order.

```cpp
SortKey key = SortKey::ForOpaque(passTag, layer, pipelineHash, linearDepth);
SortKey key = SortKey::ForTransparent(passTag, layer, linearDepth);
SortKey key = SortKey::ForUI(layer, drawOrder);
```

Typical behavior:

- opaque: grouped by pipeline and sorted front-to-back
- transparent: sorted back-to-front
- UI: sorted by explicit draw order or layer

---

## Current practical guidance

Use **semantic materials** when:

- the shader follows the engine's standard material semantics
- the material should stay backend-neutral
- the goal is flexible PBR-style authoring without exposing backend-specific bindings in gameplay or scene code

Use **explicit named parameters** when:

- the shader is custom
- the binding names are shader-specific
- a backend path is still incomplete or under debugging
- a postprocess or utility pass uses direct resource bindings

In the current engine state, semantic materials are the long-term direction, while explicit parameter materials remain important for custom passes and debugging backend-specific issues.
