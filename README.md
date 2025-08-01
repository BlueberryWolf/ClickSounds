# ClickSounds

It's exactly what it sounds like. Makes your keyboard and mouse clicks sound.. clickier.

## What it does

- **Mouse sounds**: Plays sounds for left/right/middle clicks, side buttons (X1/X2), and scroll wheel
- **Keyboard sounds**: Plays sounds when you type with multiple randomization modes
- **Audio effects**: Built-in reverb, echo, and spatial audio effects
- **Debouncing**: Prevents audio spam from key repeats and rapid scrolling
- **Fade effects**: Fade-out transitions for held keys and mouse buttons
- **Hot reload**: Configuration changes apply instantly without restarting
- **Performance optimized**: Uses async audio playback and low-level Windows hooks
- **Highly configurable**: Extensive JSON configuration with per-component controls

## Building

You'll need a C++ compiler and Premake5. The project uses clang by default.

**Using Make:**
```bash
make config=release_x64
```

**Using the batch file (Windows):**
```bash
build-clang.bat
```

**Using Premake directly:**
```bash
premake5 gmake2
make config=release_x64
```

The executable ends up in `bin/Release/ClickSounds.exe`.

## Running

**Background mode (default):**
```bash
./bin/Release/ClickSounds.exe
```

**Foreground mode (with console):**
```bash
./bin/Release/ClickSounds.exe --foreground
# or
./bin/Release/ClickSounds.exe -f
```

**Help:**
```bash
./bin/Release/ClickSounds.exe --help
```

## Configuration

The `config.json` file controls all behavior. Changes are applied instantly via hot reload.

<details>
<summary><strong>Mouse Configuration</strong></summary>

```json
"mouse": {
    "enabled": true,                    // Enable/disable all mouse sounds
    "sounds_dir": "sounds/mouse",       // Directory containing mouse sound files
    "enable_scroll_wheel": false,       // Enable sounds for scroll wheel
    "enable_side_buttons": true,        // Enable sounds for X1/X2 buttons
    "enable_fade_out": false,           // Fade out button down sounds on release
    "fade_out_duration_ms": 50,         // Fade out duration in milliseconds
    "scroll_wheel_debounce_ms": 20,     // Minimum time between scroll sounds
    "volume": 0.5,                      // Mouse volume (0.0 to 1.0)
    "sounds": {
        "left_down": "ClickDown.flac",      // Left button press
        "left_up": "ClickUp.flac",          // Left button release
        "right_down": "ClickDown2.flac",    // Right button press
        "right_up": "ClickUp2.flac",        // Right button release
        "middle_down": "ClickDownM.flac",   // Middle button press
        "middle_up": "ClickUpM.flac",       // Middle button release
        "x1_down": "ClickDown.flac",        // X1 side button press
        "x1_up": "ClickUp.flac",            // X1 side button release
        "x2_down": "ClickDown2.flac",       // X2 side button press
        "x2_up": "ClickUp2.flac",           // X2 side button release
        "wheel_up": "ClickUp.flac",         // Scroll wheel up
        "wheel_down": "ClickDown.flac"      // Scroll wheel down
    }
}
```

</details>

<details>
<summary><strong>Keyboard Configuration</strong></summary>

```json
"keyboard": {
    "enabled": true,                        // Enable/disable all keyboard sounds
    "sounds_dir": "sounds/keyboard",        // Directory containing keyboard sound files
    "random_sounds": true,                  // Use random sounds per key
    "totally_random_keypresses": true,      // True randomization mode (see below)
    "disable_repeat": false,                // Disable all key repeat sounds
    "enable_fade_out": true,                // Fade out key sounds on release
    "fade_out_duration_ms": 250,            // Fade out duration in milliseconds
    "key_repeat_debounce_ms": 50,           // Minimum time between repeat sounds
    "volume": 1.0,                          // Keyboard volume (0.0 to 1.0)
    "no_repeat_keys": [                     // Keys that don't repeat (when disable_repeat is false)
        "space", "enter", "lshift", "rshift", 
        "lctrl", "rcrtl", "lalt", "ralt", 
        "w", "a", "s", "d"
    ]
}
```

**Randomization Modes:**
- `random_sounds: false` - Each key always plays the same sound
- `random_sounds: true, totally_random_keypresses: false` - Each key gets assigned a single random sound that stays consistent
- `random_sounds: true, totally_random_keypresses: true` - Each key randomly selects a new sound every time it's pressed. Same key repeats use the same sound

</details>

<details>
<summary><strong>Audio Configuration</strong></summary>

```json
"audio": {
    "async_playback": true,             // Use asynchronous audio playback
    "max_concurrent_sounds": 32,        // Maximum simultaneous sounds
    "effects": {
        // Reverb Effect
        "enable_reverb": false,         // Enable reverb effect
        "reverb_wetness": 0.15,         // Reverb mix (0.0 = dry, 1.0 = wet)
        "reverb_room_size": 0.3,        // Room size (0.0 = small, 1.0 = large)
        "reverb_decay_time": 1.0,       // Decay time in seconds
        "reverb_damping": 0.45,         // High frequency damping
        "reverb_width": 1.0,            // Stereo width
        
        // Echo Effect
        "enable_echo": false,           // Enable echo effect
        "echo_delay": 0.2,              // Echo delay in seconds
        "echo_decay": 0.3,              // Echo volume decay (0.0-1.0)
        "echo_taps": 2,                 // Number of echo repetitions
        
        // 3D Spatial Audio
        "enable_spatializer": false,    // Enable 3D spatial positioning
        "random_spatial_position": true, // Randomize position for each sound
        "spatial_spread": 3.0,          // Width of the spatial field
        "listener_distance": 1.5        // Distance from listener to sound field
    }
}
```

</details>

### Quick Setup Examples

**Gaming setup** (minimal latency, no effects):
```json
{
    "mouse": { "enabled": true, "enable_scroll_wheel": false, "volume": 0.3 },
    "keyboard": { "enabled": true, "totally_random_keypresses": false, "volume": 0.5 },
    "audio": { "max_concurrent_sounds": 16, "effects": { "enable_reverb": false } }
}
```

**Immersive setup** (with effects):
```json
{
    "mouse": { "enabled": true, "enable_fade_out": true, "volume": 0.7 },
    "keyboard": { "enabled": true, "enable_fade_out": true, "volume": 0.8 },
    "audio": { 
        "effects": { 
            "enable_reverb": true, "reverb_wetness": 0.2,
            "enable_spatializer": true, "spatial_spread": 2.0
        }
    }
}
```

## Sound Files

Sounds are organized in the `sounds/` directory:
- `sounds/mouse/` - Mouse click sounds (6 files included)
- `sounds/keyboard/` - Keyboard sounds (12 files included)

The app uses FLAC files by default for good quality and small size. You can add your own sounds - just drop them in the appropriate folder and update the config.

## Features in Detail

### Hot Reload
Configuration changes are applied instantly without restarting the application. Just edit `config.json` and save.

### Smart Debouncing
- **Key repeat debouncing**: Prevents audio spam from held keys
- **Scroll wheel debouncing**: Configurable delay between scroll sounds
- **Per-key repeat control**: Specify which keys should never repeat

### Audio Effects
- **Reverb**: Simulates room acoustics with configurable parameters
- **Echo**: Multi-tap echo with adjustable delay and decay
- **Spatializer**: it positions audio in 3d and supports randomization for "immersion"

### Performance Optimizations
- **Async audio playback**: Sounds don't block input processing
- **Low-level Windows hooks**: No CPU-intensive polling
- **Concurrent sound limiting**: Prevents audio system overload
- **Smart cleanup**: Automatically manages audio resources

## Dependencies

- **miniaudio**: Cross-platform audio playback library
- **nlohmann/json**: Modern C++ JSON library for configuration
- **FileWatch**: Cross-platform file monitoring for hot reload
- **Windows API**: Low-level input monitoring (Windows only currently)

## Platform Support

Currently Windows-only due to the input monitoring implementation. Linux support is planned for future releases.

## Performance

<<<<<<< HEAD
This thing is actually fast (unlike the older AHK script by [@Camera4040](https://github.com/camera4040)). Uses Windows low-level hooks for input monitoring (no polling nonsense) and async audio playback so sounds don't block your typing. It is configured by default to handle 32 concurrent sounds without breaking a sweat.
=======
This application is designed to be lightweight and responsive:
- Uses Windows low-level hooks (no polling)
- Async audio playback prevents input lag
- Configurable concurrent sound limits (default: 32)
- Minimal CPU usage even during intensive typing/gaming
>>>>>>> d807eff (big changes!! hot reloading, effects, idk)

Won't interfere with your gaming performance or fast typing.

## Why?

Because mechanical keyboards sound cool and make you feel productive, so now you can annoy everyone in your screenshare with maximum efficiency.

## License

<<<<<<< HEAD
Do whatever you want with it. If it breaks your computer that's on you lmao
=======
Public domain. Do literally whatever you want with it. If it breaks your computer that's on you lmao
>>>>>>> d807eff (big changes!! hot reloading, effects, idk)
