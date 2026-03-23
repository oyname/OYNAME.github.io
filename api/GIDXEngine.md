# GIDXEngine

Top-Level Runtime-Klasse des OYNAME Engine. Koordiniert Fenster, Renderer und den Haupt-Frame-Loop.

## Deklaration

```cpp
class GIDXEngine
{
public:
    GIDXEngine();
    ~GIDXEngine();

    bool Init(const WindowDesc& desc);
    void Run();
    void Shutdown();

    GDXECSRenderer* GetRenderer();
    IGDXWindow*     GetWindow();
};
```

## Methoden

### `Init()`

```cpp
bool Init(const WindowDesc& desc);
```

Initialisiert Engine, Fenster, DX11-Device und Renderer. Gibt `false` zurück bei einem Fehler.

---

### `Run()`

```cpp
void Run();
```

Startet den Haupt-Loop. Blockiert bis das Fenster geschlossen wird. Ruft intern `PollEvents()`, Delta-Time-Berechnung und `Render()` auf.

---

### `Shutdown()`

```cpp
void Shutdown();
```

Gibt alle Ressourcen frei. Wird automatisch im Destruktor aufgerufen.

---

### `GetRenderer()`

```cpp
GDXECSRenderer* GetRenderer();
```

Gibt einen Zeiger auf den aktiven Renderer zurück. Nur nach erfolgreichem `Init()` gültig.

## Verwendungsbeispiel

```cpp
#include "gidx.h"

int main()
{
    WindowDesc desc;
    desc.width  = 1280;
    desc.height = 720;
    desc.title  = "OYNAME App";

    GIDXEngine engine;

    if (!engine.Init(desc))
        return -1;

    // Ressourcen laden, Szene aufbauen...
    GDXECSRenderer* renderer = engine.GetRenderer();

    engine.Run();
    return 0;
}
```

## Hinweise

- `Run()` ist blockierend. Eigene Logik wird über Callback-Hooks oder vor dem `Run()`-Aufruf registriert.
- Bei der direkten Engine-Path werden Fenster, Adapter und Backend manuell erstellt — `GIDXEngine` ist nur für den vereinfachten Pfad via `gidx.h`.

## Siehe auch

- [GDXECSRenderer](GDXECSRenderer)
- [IGDXWindow](IGDXWindow)
