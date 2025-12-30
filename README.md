# TmOS
A *simple* operating system for the M5Stack Cardputer(-ADV).

## How to use?
 - Clone the repo
 - Compile and upload using PlatformIO
 - Follow special instructions for some apps in order to make them work properly.

## Some tips
 - Most of the apps require an SD card (formatted in FAT32/FAT. exFAT won't work)
  - Some apps require specific files on the SD card. Check the apps' instructions for more info.

## App Instructions

### Music
- Place `.mp3` files in `/music` or anywhere on the SD card; both locations are scanned.
- Use the naming pattern `Artist-Album-Title.mp3` to get artist/album grouping in the list. Files without this pattern are grouped as “Uncategorized”.
- Optional lyrics: put a `.lrc` file next to the `.mp3` with the exact same name (e.g. `Song.mp3` + `Song.lrc`). Timestamps like `[mm:ss.xx]` are supported.
- Controls: move through the list and select a track; in the player view use the slider for volume, and press `Z` to increase or `X` to decrease volume.
- Playback auto-advances to the next song when one finishes.
- Only `.mp3` is supported.

Example `.lrc` snippet:
```
[00:12.00] Line before
[00:15.20] Current line
[00:18.50] Next line
```

### Pictures
- Supported format: LVGL raw image `.bin` in `RGB565` true color (no alpha). Use the LVGL Image Converter and export “Binary” with color format `RGB565`.
- File naming: visible gallery items must be prefixed `i_` and not start with `iec_` (e.g. `i_mountain.bin`). Files named `iec_...` are reserved for password-locked viewing.
- You can place images anywhere on the SD card; the app scans all folders for `.bin`.
- Viewer controls:
  - Zoom in `=` and Zoom out `-`
  - Pan Up `;`, Down `.`, Left `,`, Right `/`
  - Toolbar buttons also provide zoom, fit-to-window, and pan.
- Password-locked image:
  - Tap “Decrypt Image”, enter your password.
  - The app computes `MD5(password).upper()[8:24]` and looks for a file named `iec_<SUBSTRING>.bin` in SD root. If present, it opens it.
  - To prepare such a file: convert your `.bin` image as usual and rename it to `iec_<SUBSTRING>.bin` using the rule above.

### UX Executor
- Place `.uxc` files (JSON) anywhere on the SD card; they will be listed and can be launched.
- Format: a JSON object with a `widgets` array; each item describes a widget. Basic types include `label`, `button`, and `img`. Buttons can define `on.click` actions like `toast`.
- Images used by `img.src` should point to LVGL `.bin` images as described in Pictures.
- You can author `.uxc` files directly with the built-in UX Editor.

Minimal `.uxc` example:
```json
{
  "widgets": [
    { "type": "label", "text": "Hello UXC" },
    { "type": "button", "text": "Click",
      "on": { "click": { "action": "toast", "text": "Clicked!" } }
    }
  ]
}
```
