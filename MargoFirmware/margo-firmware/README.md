# Margo Firmware

Margo is a clean-room M5Stack StickS3 firmware starter inspired by Bruce's modular app flow: a small launcher, reusable feature modules, and hardware-first navigation.

This project does not patch or redistribute the `Bruce-m5stack-sticks3.bin` image. If you later copy code from Bruce itself, keep the upstream license obligations intact. Bruce firmware is AGPL-licensed, so derivative source needs to stay open under compatible terms.

## Current Features

- Margo-branded launcher UI for M5Stack StickS3
- App registry pattern for adding modules without rewriting the menu
- Device status page with battery, uptime, heap, and PSRAM info
- Passive Wi-Fi scan that lists visible networks only
- IR hardware page with StickS3 power-enable handling and safe placeholder behavior
- Settings page for brightness and external 5V output

## Controls

- `BtnA`: open/select
- `BtnB`: next item
- `BtnC`: back, when available
- Hold `BtnA` on most screens to return to the launcher

## Build

Install PlatformIO, then from this folder:

```powershell
pio run
```

On this machine, PlatformIO was also installed into Codex's bundled Python. You can rebuild and regenerate the merged image with:

```powershell
.\scripts\build.ps1
```

Flash to a StickS3:

```powershell
pio run -t upload
```

The merged binary is written to:

```text
dist\Margo-m5stack-sticks3.bin
```

Monitor serial output:

```powershell
pio device monitor
```

StickS3 may need download mode: hold the side reset button for about two seconds until the internal green LED flashes, then upload.

## Safety Note

Keep Margo for devices, networks, and lab setups you own or have permission to test. The starter intentionally avoids deauth, jamming, credential capture, and payload injection code.
