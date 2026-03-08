# burglar_alarm_v2_final

This repository contains a burglar alarm project with two main parts:

- `alarm_v2/arduino/` — Arduino/C++ code for sensors, buzzer, solenoid, and the main alarm firmware.
- `alarm_v2/python/` — Python utilities for face detection, serial communication with the Arduino, and notification services.

Quick start
1. Arduino: open `alarm_v2/arduino/BurglarAlarm/BurglarAlarm.ino` in the Arduino IDE and upload to your board.
2. Python: create a virtual environment, install dependencies, then run `main.py`.

Example (Linux):
```bash
python -m venv .venv
source .venv/bin/activate
pip install -r alarm_v2/python/requirements.txt
python alarm_v2/python/main.py
```

License
This project is licensed under the MIT License — see `LICENSE`.

Contributing
- Open issues or PRs describing bug fixes, improvements, or feature requests.

Contact
If you want help setting up the hardware or running the code, open an issue with details about your OS and board.
