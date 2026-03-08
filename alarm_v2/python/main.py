"""
main.py  -  Entry point for the PC-side burglar alarm bridge.

Usage:
    python main.py --port COM3           (Windows)
    python main.py --port /dev/ttyACM0   (Linux / Mac)

Leave this terminal open while the system is running.
Type commands and press Enter to send them to the Arduino.
"""
import argparse
import logging
import os
import signal
import sys
import threading
import time
from serial_comm import SerialComm

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(name)s - %(message)s",
    handlers=[
        logging.StreamHandler(sys.stdout),
        logging.FileHandler("alarm.log"),
    ],
)
logger = logging.getLogger("main")

BANNER = """
+--------------------------------------------------+
|     Burglar Alarm System  -  PC Control Panel    |
|     Arduino Mega 2560  +  Logitech C270          |
+--------------------------------------------------+
|  Commands (type here + press Enter):             |
|   ARM_FULL                arm all sensors        |
|   ARM_HOME                arm door/window only   |
|   DISARM:<pin>            disarm (e.g. 123456)   |
|   CANCEL_ALARM:<pin>      stop buzzer instantly   |
|   CHANGE_PIN:<old>:<new>  update PIN              |
|   ENROL_FACE              enrol a face via C270   |
|   SET_TIMEOUT:<sec>       set entry delay 5-120   |
|   STATUS                  query current mode      |
|   quit                    exit this program       |
+--------------------------------------------------+
"""


def keyboard_thread(comm: SerialComm) -> None:
    """Reads keyboard input and forwards commands to the Arduino."""
    while True:
        try:
            cmd = input("").strip()
            if cmd.lower() in ("quit", "exit", "q"):
                os._exit(0)
            if cmd:
                comm.send(cmd)
        except (EOFError, KeyboardInterrupt):
            os._exit(0)


def main() -> None:
    parser = argparse.ArgumentParser(description="Burglar Alarm PC Bridge")
    parser.add_argument("--port", default="COM3",
                        help="Serial port (COM3 / /dev/ttyACM0)")
    parser.add_argument("--baud", type=int, default=9600,
                        help="Baud rate (default 9600)")
    args = parser.parse_args()

    print(BANNER)

    comm = SerialComm(port=args.port, baud=args.baud)

    def shutdown(sig, frame):
        print("\nShutting down...")
        comm.stop()
        sys.exit(0)

    signal.signal(signal.SIGINT,  shutdown)
    signal.signal(signal.SIGTERM, shutdown)

    if not comm.connect():
        print(f"\nERROR: Could not open {args.port}.")
        print("Check the Arduino is connected and the port is correct.")
        sys.exit(1)

    comm.start()
    print(f"Connected on {args.port}. Waiting for Arduino...\n")

    kb_thread = threading.Thread(target=keyboard_thread, args=(comm,), daemon=True)
    kb_thread.start()

    while True:
        time.sleep(1)


if __name__ == "__main__":
    main()
