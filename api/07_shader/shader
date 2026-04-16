# KROM Engine Shader API Reference

This document describes the shader system used by the engine, based on the current project source.

## Overview

The engine uses its **own shader assets, shader runtime, shader compiler pipeline, and binding model**.
It does not rely on a single universal shader file per material. Instead, it organizes shader logic around:

- CPU-side shader assets
- backend-specific compiled artifacts
- runtime shader preparation
- shader variants driven by feature flags
- a fixed engine-wide binding contract

In practice, one logical shader can exist in multiple source files and target formats for different backends.

---

## Shader Source Files

The project contains shader source files under `assets/` such as:

- `pbr_lit.hlslvs`
- `pbr_lit.hlslps`
- `pbr_lit.vert`
- `pbr_lit.frag`
- `pbr_lit_vk.vert`
- `pbr_lit_vk.frag`
- `quad_unlit.*`
- `triangle_color.*`
- `fullscreen.*`
- `passthrough.*`

### Naming Convention

The names encode three things:

1. **Logical shader purpose**
   - `pbr_lit`
   - `quad_unlit`
   - `triangle_color`
   - `fullscreen`
   - `passthrough`

2. **Backend or target path**
   - no suffix: default path, usually OpenGL-style GLSL
   - `_vk`: Vulkan-target GLSL/SPIR-V path

3. **Shader stage and language**
   - `.hlslvs` = HLSL vertex shader
   - `.hlslps` = HLSL pixel shader
   - `.vert` = GLSL vertex shader
   - `.frag` = GLSL fragment shader

This means the engine uses **multiple physical files for one logical shader effect**.

---

## Core Shader Asset Types

Defined in `include/assets/AssetRegistry.hpp`.

### `enum class ShaderStage`

Represents the stage of a shader asset.

Values:

- `Vertex`
- `Fragment`
- `Compute`
- `Geometry`
- `Hull`
- `Domain`

### `enum class ShaderSourceLanguage`

Represents the authoring language of the shader source.

Values:

- `Unknown`
- `HLSL`
- `GLSL`
- `WGSL`

### `enum class ShaderTargetProfile`

Represents the concrete runtime compilation target.

Values:

- `Generic`
- `Null`
- `DirectX11_SM5`
- `DirectX12_SM6`
- `Vulkan_SPIRV`
- `OpenGL_GLSL450`

This is the reason the engine needs several shader files and compiled outputs: the target profile is backend-specific.

### `struct CompiledShaderArtifact`

Represents a compiled or prepared shader result for a specific target profile.

Key fields:

- `target` — the target backend/profile
- `stage` — shader stage
- `entryPoint` — entry function, default `main`
- `debugName` — readable identifier
- `bytecode` — binary output when used
- `sourceText` — textual shader output when used
- `sourceHash` — hash of the logical source state
- `defines` — active variant defines
- `cacheKey` — cache lookup key
- `cacheSchemaVersion` — cache format version
- `dependencies` — tracked include/input dependencies
- `contract` — shader pipeline contract used by runtime/bindings

Method:

- `bool IsValid() const noexcept`
  - Returns `true` if either `bytecode` or `sourceText` is present.

### `struct ShaderAsset`

Represents the CPU-side logical shader asset.

Key fields:

- `stage`
- `sourceLanguage`
- `entryPoint`
- `sourceCode` — logical source code
- `resolvedPath` — resolved file path used for includes/cache
- `bytecode` — legacy fallback or precompiled data
- `compiledArtifacts` — target-specific compiled outputs
- `gpuStatus`

Interpretation:

A `ShaderAsset` is the authoring/runtime source object. A `CompiledShaderArtifact` is the target-specific output generated from it.

---

## Shader Compiler API

Defined in `include/renderer/ShaderCompiler.hpp`.

## `class ShaderCompiler`

Static utility API for backend target resolution and shader compilation.

### `static assets::ShaderTargetProfile ResolveTargetProfile(const IDevice& device)`

Maps the active device/backend to a shader target profile.

Typical purpose:

- DX11 device -> `DirectX11_SM5`
- Vulkan device -> `Vulkan_SPIRV`
- OpenGL device -> `OpenGL_GLSL450`

### `static ShaderTargetApi ResolveTargetApi(const IDevice& device)`

Returns the abstract shader API family for the active device.

### `static ShaderBinaryFormat ResolveBinaryFormat(assets::ShaderTargetProfile profile) noexcept`

Maps a target profile to the expected output representation.

### `static const char* ToString(assets::ShaderTargetProfile profile) noexcept`

Returns a readable string name for the target profile.

### `static bool IsRuntimeConsumable(const assets::CompiledShaderArtifact& shader) noexcept`

Checks whether a compiled artifact can be consumed by the runtime.

### `static bool CompileForTarget(const assets::ShaderAsset& asset, assets::ShaderTargetProfile target, assets::CompiledShaderArtifact& outCompiled, std::string* outError = nullptr)`

Compiles one logical shader asset for one concrete target profile.

### `static std::vector<std::string> VariantFlagsToDefines(ShaderVariantFlag flags) noexcept`

Converts engine-neutral variant bits into concrete preprocessor symbols.

Examples from the current implementation:

- `Skinned` -> `KROM_SKINNING`
- `VertexColor` -> `KROM_VERTEX_COLOR`
- `AlphaTest` -> `KROM_ALPHA_TEST`
- `NormalMap` -> `KROM_NORMAL_MAP`
- `Unlit` -> `KROM_UNLIT`
- `ShadowPass` -> `KROM_SHADOW_PASS`
- `Instanced` -> `KROM_INSTANCED`
- `BaseColorMap` -> `KROM_BASECOLOR_MAP`
- `MetallicMap` -> `KROM_METALLIC_MAP`
- `RoughnessMap` -> `KROM_ROUGHNESS_MAP`
- `OcclusionMap` -> `KROM_OCCLUSION_MAP`
- `EmissiveMap` -> `KROM_EMISSIVE_MAP`
- `OpacityMap` -> `KROM_OPACITY_MAP`
- `PBRMetalRough` -> `KROM_PBR_METAL_ROUGH`
- `DoubleSided` -> `KROM_DOUBLE_SIDED`
- `ORMMap` -> `KROM_ORM_MAP`
- `IBLMap` -> `KROM_IBL`

### `static bool CompileVariant(const assets::ShaderAsset& asset, assets::ShaderTargetProfile target, ShaderVariantFlag flags, assets::CompiledShaderArtifact& outCompiled, std::string* outError = nullptr)`

Compiles a specific shader variant from one logical shader asset and one active variant bitset.

---

## Shader Variant Model

Defined in `include/renderer/RendererTypes.hpp`.

### `enum class ShaderVariantFlag : uint32_t`

Bitfield describing optional shader features.

Values:

- `None`
- `Skinned`
- `VertexColor`
- `AlphaTest`
- `NormalMap`
- `Unlit`
- `ShadowPass`
- `Instanced`
- `BaseColorMap`
- `MetallicMap`
- `RoughnessMap`
- `OcclusionMap`
- `EmissiveMap`
- `OpacityMap`
- `PBRMetalRough`
- `DoubleSided`
- `ORMMap`
- `IBLMap`

### `bool HasFlag(ShaderVariantFlag flags, ShaderVariantFlag bit) noexcept`

Helper used to test feature bits.

### `enum class ShaderPassType : uint8_t`

Logical pass classification.

Values:

- `Main`
- `Shadow`
- `Depth`
- `UI`

### `struct ShaderVariantKey`

Key for variant cache lookup.

Fields:

- `baseShader`
- `pass`
- `flags`

Methods:

- `ShaderVariantKey Normalized() const noexcept`
- `uint64_t Hash() const noexcept`

Interpretation:

A variant is not only defined by feature flags, but also by the logical pass type.

---

## Shader Runtime API

Defined in `include/renderer/ShaderRuntime.hpp`.

## `class ShaderRuntime`

Central runtime subsystem that prepares shader assets, material shader state, pipeline state, and resource bindings for rendering.

### Lifecycle

#### `bool Initialize(IDevice& device)`
Initializes the runtime for the active device.

#### `void Shutdown()`
Releases runtime-owned state.

#### `void SetAssetRegistry(assets::AssetRegistry* registry) noexcept`
Assigns the asset registry used for shader/material asset lookup.

#### `assets::AssetRegistry* GetAssetRegistry() const noexcept`
Returns the currently bound asset registry.

### Shader Preparation

#### `bool CollectShaderRequests(const MaterialSystem& materials, std::vector<ShaderHandle>& outRequests) const`
Collects shader assets required by current materials.

#### `ShaderHandle PrepareShaderAsset(ShaderHandle shaderAssetHandle)`
Prepares one shader asset for GPU/runtime use.

#### `bool CommitShaderRequests(const std::vector<ShaderHandle>& requests)`
Processes a list of pending shader requests.

#### `bool PrepareAllShaderAssets()`
Prepares all known shader assets.

### Material Preparation

#### `bool CollectMaterialRequests(const MaterialSystem& materials, std::vector<MaterialHandle>& outRequests) const`
Collects materials requiring runtime shader/pipeline state.

#### `bool PrepareMaterial(const MaterialSystem& materials, MaterialHandle material)`
Builds runtime state for one material.

#### `bool CommitMaterialRequests(const MaterialSystem& materials, const std::vector<MaterialHandle>& requests)`
Processes pending material preparation requests.

#### `bool PrepareAllMaterials(const MaterialSystem& materials)`
Prepares all materials.

### Query API

#### `const MaterialGpuState* GetMaterialState(MaterialHandle material) const noexcept`
Returns the prepared GPU-facing material state.

#### `const ShaderAssetStatus* GetShaderStatus(ShaderHandle shaderAssetHandle) const noexcept`
Returns preparation/compilation status of a shader asset.

#### `ShaderHandle GetOrCreateVariant(ShaderHandle shaderAssetHandle, ShaderPassType pass, ShaderVariantFlag flags)`
Returns an existing variant or builds a new one for the given pass and flags.

#### `const ShaderVariantCache& GetVariantCache() const noexcept`
Returns the runtime variant cache.

### Binding API

#### `bool BindMaterial(...)`
Legacy binding overload using full buffers.

#### `bool BindMaterialWithRange(...)`
Preferred binding overload using `BufferBinding` ranges with offset/size support.

This is the modern path for arena-based per-object and per-pass constant buffer binding.

### Validation API

#### `bool ValidateMaterial(const MaterialSystem& materials, MaterialHandle material, std::vector<ShaderValidationIssue>& outIssues) const`
Validates whether the current material and shader state are consistent.

### Environment API

#### `void SetEnvironmentState(const EnvironmentRuntimeState& state) noexcept`
Sets environment/IBL state used by shader binding and variant evaluation.

#### `const EnvironmentRuntimeState& GetEnvironmentState() const noexcept`
Returns current environment state.

#### `bool HasIBL() const noexcept`
Returns whether image-based lighting is active.

### Statistics

#### `size_t PreparedShaderCount() const noexcept`
Number of prepared shader assets.

#### `size_t PreparedMaterialCount() const noexcept`
Number of prepared materials.

#### `bool IsRenderThread() const noexcept`
Whether the calling thread is the registered render thread.

---

## Shader Runtime Status Types

### `struct ShaderValidationIssue`

Represents a warning or error reported by runtime validation.

Fields:

- `severity`
- `message`

### `struct ShaderAssetStatus`

Describes the runtime state of one shader asset.

Fields:

- `assetHandle`
- `gpuHandle`
- `stage`
- `target`
- `contract`
- `compiledHash`
- `loaded`
- `fromBytecode`
- `fromCompiledArtifact`

### `struct ResolvedMaterialBinding`

Represents a concrete resolved binding used for one prepared material.

Kinds:

- `ConstantBuffer`
- `Texture`
- `Sampler`

Fields:

- `kind`
- `name`
- `slot`
- `stages`
- `texture`
- `samplerIndex`

### `struct MaterialGpuState`

Represents the GPU/runtime state of a prepared material.

Fields:

- `material`
- `pipeline`
- `vertexShader`
- `fragmentShader`
- `perMaterialCB`
- `perMaterialCBSize`
- `contentHash`
- `materialRevision`
- `environmentRevision`
- `valid`
- `bindings`
- `issues`

---

## Shader Binding Model

Defined in `include/renderer/ShaderBindingModel.hpp`.

The engine uses a **fixed explicit binding contract** across backends.
This is important: shader code and runtime code are expected to agree on these slot indices.

### Constant Buffer Slots

`struct CBSlots`

- `PerFrame = 0`
- `PerObject = 1`
- `PerMaterial = 2`
- `PerPass = 3`

### Texture Slots

`struct TexSlots`

- `Albedo = 0`
- `Normal = 1`
- `ORM = 2`
- `Emissive = 3`
- `ShadowMap = 4`
- `IBLIrradiance = 5`
- `IBLPrefiltered = 6`
- `BRDFLUT = 7`
- `PassSRV0 = 8`
- `PassSRV1 = 9`
- `PassSRV2 = 10`
- `HistoryBuffer = 11`
- `BloomTexture = 12`

### Sampler Slots

`struct SamplerSlots`

- `LinearWrap = 0`
- `LinearClamp = 1`
- `PointClamp = 2`
- `ShadowPCF = 3`

### UAV Slots

`struct UAVSlots`

- `Output0 = 0`
- `Output1 = 1`
- total range size: `COUNT = 8`

### Register Mapping

`struct BindingRegisterRanges`

Provides linear register ranges for APIs that need explicit descriptor/register mapping.

Bases:

- constant buffers start at `0`
- shader resources start at `16`
- samplers start at `32`
- UAVs start at `48`

Helpers:

- `CB(slot)`
- `SRV(slot)`
- `SMP(slot)`
- `UAV(slot)`

Interpretation:

The engine distinguishes between **logical slots** and **backend register ranges**, but both are part of one engine-wide contract.

---

## Shader Contract Types

Defined in `include/renderer/ShaderContract.hpp`.

These types describe the interface and binding contract produced during compilation and consumed at runtime.

### `struct ShaderPipelineContract`

High-level contract for one shader pipeline result.

Fields:

- `api`
- `binaryFormat`
- `stageMask`
- `interfaceLayout`
- `pipelineBinding`
- `pipelineStateKey`
- `contractHash`

Method:

- `bool IsValid() const noexcept`

### `using PipelineLayoutContract = PipelineBindingContract`

Alias for pipeline layout/binding contract use.

Interpretation:

This contract is the bridge between shader compilation output and runtime binding/pipeline creation.

---

## Why There Are So Many Shader Files

The engine uses many shader files because it separates concerns that other systems often try to hide in one file:

- **logical effect name**
- **shader stage**
- **source language**
- **target backend/profile**
- **runtime variant feature set**

One logical material shader may therefore exist as:

- HLSL vertex stage
- HLSL pixel stage
- GLSL vertex stage
- GLSL fragment stage
- Vulkan-specific GLSL vertex stage
- Vulkan-specific GLSL fragment stage
- multiple runtime variants via feature defines

So the naming may look unusual, but it is technically consistent with a multi-backend shader pipeline.

---

## Practical Summary

The engine shader system consists of these main layers:

1. **Shader source files** in `assets/`
2. **CPU shader assets** via `ShaderAsset`
3. **Target-specific outputs** via `CompiledShaderArtifact`
4. **Compilation and target resolution** via `ShaderCompiler`
5. **Variant generation** via `ShaderVariantFlag` and `CompileVariant`
6. **Runtime preparation and binding** via `ShaderRuntime`
7. **Fixed slot contract** via `ShaderBindingModel.hpp`
8. **Pipeline/interface contract** via `ShaderContract.hpp`

That is why the engine uses its own shaders, why there are many of them, and why the file names encode backend and stage information so explicitly.
