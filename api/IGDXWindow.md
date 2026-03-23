# IGDXWindow

Plattformneutrales Interface für Fenster-Implementierungen. Alle Fenstertypen leiten von dieser Klasse ab.

## Deklaration

```cpp
class IGDXWindow
{
public:
    virtual ~IGDXWindow() = default;

    virtual void        PollEvents() = 0;
    virtual bool        ShouldClose() const = 0;
    virtual int         GetWidth() const = 0;
    virtual int         GetHeight() const = 0;
    virtual bool        GetBorderless() const = 0;
    virtual const char* GetTitle() const = 0;
    virtual void        SetTitle(const char* title) = 0;
};
```

## Methoden

### `PollEvents()`

```cpp
virtual void PollEvents() = 0;
```

Verarbeitet alle ausstehenden OS-Nachrichten. Muss einmal pro Frame aufgerufen werden.

---

### `ShouldClose()`

```cpp
virtual bool ShouldClose() const = 0;
```

Gibt `true` zurück, wenn das Fenster geschlossen werden soll (z.B. durch Alt+F4 oder Schließen-Button).

---

### `GetWidth()` / `GetHeight()`

```cpp
virtual int GetWidth() const = 0;
virtual int GetHeight() const = 0;
```

Gibt die aktuelle Fensterauflösung in Pixeln zurück.

---

### `SetTitle()`

```cpp
virtual void SetTitle(const char* title) = 0;
```

Ändert den Fenstertitel zur Laufzeit. Nützlich für FPS-Anzeigen oder Zustandsanzeigen.

## Verwendungsbeispiel

```cpp
IGDXWindow* window = ...; // z.B. GDXWin32Window

while (!window->ShouldClose())
{
    window->PollEvents();
    // Frame-Logik hier
}
```

## Implementierungen

- `GDXWin32Window` — Standard Win32-Implementierung

## Siehe auch

- [WindowDesc](WindowDesc)
- [GDXWin32Window](GDXWin32Window)
