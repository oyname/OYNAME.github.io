# WindowDesc

Konfigurationsstruktur für das Win32-Fenster. Wird beim Erstellen eines `GDXWin32Window` übergeben.

## Deklaration

```cpp
struct WindowDesc
{
    int         width      = 1280;
    int         height     = 720;
    std::string title      = "GDX";
    bool        resizable  = true;
    bool        borderless = true;
};
```

## Felder

| Feld | Typ | Standard | Beschreibung |
|------|-----|---------|-------------|
| `width` | `int` | `1280` | Fensterbreite in Pixeln |
| `height` | `int` | `720` | Fensterhöhe in Pixeln |
| `title` | `std::string` | `"GDX"` | Fenstertitel in der Titelleiste |
| `resizable` | `bool` | `true` | Ob das Fenster skalierbar ist |
| `borderless` | `bool` | `true` | Ob das Fenster randlos ist |

## Verwendungsbeispiel

```cpp
WindowDesc desc;
desc.width      = 1920;
desc.height     = 1080;
desc.title      = "Meine Anwendung";
desc.resizable  = false;
desc.borderless = true;

GDXWin32Window window(desc, eventQueue);
```

## Hinweise

- `borderless = true` entfernt die Windows-Titelleiste vollständig. Das Fenster kann dann nur durch eigene UI-Logik verschoben werden.
- Wird `resizable = false` gesetzt, ignoriert das Betriebssystem Resize-Events.

## Siehe auch

- [GDXWin32Window](GDXWin32Window)
- [IGDXWindow](IGDXWindow)
