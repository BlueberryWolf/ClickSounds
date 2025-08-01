# ClickSounds

Yeah, it's exactly what it sounds like. Makes your keyboard and mouse clicks sound.. clickier.

## What it does

- Plays sounds when you click your mouse
- Plays sounds when you type on your keyboard  
- Has a bunch of different click sounds so it's not completely repetitive
- Configurable because I got tired of hardcoding everything
- Actually performant (uses async audio playback and low-level Windows hooks so it doesn't slow down your typing)

## Building

You'll need a C++ compiler. I chose clang but whatever works.

```bash
make
```

Or if you're on Windows and want to use the batch file:
```bash
build-clang.bat
```

The executable ends up in `bin/Release/ClickSounds.exe`.

## Running

Just run the exe. It'll sit in the background and make noise when you type/click.

```bash
./bin/Release/ClickSounds.exe
```

## Configuration

Edit `config.json` if you want to change stuff:

- `mouse.enabled` (bool) - turn mouse sounds on/off
- `keyboard.enabled` (bool) - turn keyboard sounds on/off  
- `keyboard.random_sounds` (bool) - picks random sounds instead of the same one
- `keyboard.disable_repeat` (bool) - completely disables repeat sounds for all keys when held down
- `keyboard.no_repeat_keys` (array) - specific keys that won't make noise when held down (only works when disable_repeat is false)
- `audio.max_concurrent_sounds` (int) - how many sounds can play at once

## Sounds

Sounds go in the `sounds/` folder:
- `sounds/mouse/` - mouse click sounds
- `sounds/keyboard/` - keyboard sounds

Currently using FLAC files because they're small and don't sound terrible.

## Dependencies

- miniaudio for audio playback
- nlohmann/json for config parsing
- Windows API for input monitoring (sorry Linux users, maybe later)

## Performance

This thing is actually fast. Uses Windows low-level hooks for input monitoring (no polling nonsense) and async audio playback so sounds don't block your typing. Is configured by default to handle 32 concurrent sounds without breaking a sweat.

Basically it won't slow down your fast ass typing or mess with your gaming.

## Why?

Because mechanical keyboards sound cool and make you feel productive so now you can annoy everyone with it in your screenshare.

## License

Do whatever you want with it. If it breaks your computer that's on you lmao