# Garden Watering System

Web UI + Python bridge to control an Arduino-based garden system.

## Project Structure
- `index.html`, `style.css`, `script.js` – Web frontend
- `server.py` – Python Flask bridge (talks to Arduino over Serial)
- `arduino_garden.ino` – Arduino sketch (LCD + moisture + manual WATER/FERTILIZE)
- `requirements.txt` – Python dependencies

## Prerequisites
- Windows: Python 3.12+ (winget install Python.Python.3.12)
- Arduino IDE (or Arduino CLI)
- USB driver (CH340/CH341 if using a clone board)

## Quick Start (Windows, Git Bash)
```bash
# 1) Clone
git clone https://github.com/Rolando-web/IT10.git
cd IT10

# 2) Python venv + deps
"/c/Users/$USERNAME/AppData/Local/Programs/Python/Python312/python.exe" -m venv .venv
source .venv/Scripts/activate
pip install -r requirements.txt

# 3) Run the bridge (COM port can be overridden)
# Defaults to COM4. To change, set ARDUINO_PORT env var.
# Example: ARDUINO_PORT=COM5 python server.py
python server.py
```

- Server runs on: `http://127.0.0.1:8000`
- Status: `http://127.0.0.1:8000/status`
- Commands: `/command?cmd=water` and `/command?cmd=fertilize`
- Schedule store: `/setdate?date=YYYY-MM-DD&time=HH:MM` (stored only)

## Arduino Upload
1. Open `arduino_garden.ino` in Arduino IDE
2. Tools → Board: select your board (e.g., Arduino Uno)
3. Tools → Port: select your COM port (e.g., COM4)
4. Upload

> If you see “Missing FQBN”, select the correct Board and install its core via Boards Manager (e.g., Arduino AVR Boards for Uno/Nano/Mega).

## Changing the COM Port
- The bridge prefers `COM4` by default. Override with env var:
  - PowerShell:
    ```powershell
    $env:ARDUINO_PORT="COM5"
    .\.venv\Scripts\python.exe .\server.py
    ```
  - Git Bash:
    ```bash
    ARDUINO_PORT=COM5 "/d/IT10/.venv/Scripts/python.exe" /d/IT10/server.py
    ```

## Notes
- The Arduino sketch shows only DRY/WET on LCD and does not auto-water.
- Manual control: Use web buttons (Water/Fertilize) or Serial commands (WATER/FERTILIZE).
- Time in the UI is Asia/Manila (UTC+8).
