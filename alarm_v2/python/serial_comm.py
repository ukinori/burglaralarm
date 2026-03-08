"""
serial_comm.py  -  SerialComm class
Bidirectional USB serial bridge between the PC and Arduino Mega 2560.
Dispatches Arduino commands to FaceDetection and NotificationService.
"""
import serial
import threading
import logging
import csv
import datetime
import time
import os
from face_detection import FaceDetection
from notification_service import NotificationService

logger  = logging.getLogger(__name__)
LOG_CSV = "intrusion_log.csv"


class SerialComm:
    """
    Maintains the serial connection to the Arduino and routes messages.

    Parameters
    ----------
    port : str   Serial port (e.g. COM3 or /dev/ttyACM0)
    baud : int   Baud rate — must match Arduino (default 9600)
    """

    def __init__(self, port: str, baud: int = 9600):
        self._port      = port
        self._baud      = baud
        self._ser       = None
        self._running   = False
        self._thread    = None
        self._face      = FaceDetection()
        self._notifier  = NotificationService()
        self._init_csv()

    # ── CSV log ───────────────────────────────────────────────
    def _init_csv(self) -> None:
        if not os.path.exists(LOG_CSV):
            with open(LOG_CSV, "w", newline="") as f:
                csv.writer(f).writerow(["timestamp", "event"])

    def _log_csv(self, event: str) -> None:
        ts = datetime.datetime.now().isoformat()
        with open(LOG_CSV, "a", newline="") as f:
            csv.writer(f).writerow([ts, event])

    # ── Connection ────────────────────────────────────────────
    def connect(self) -> bool:
        """Open the serial port to the Arduino."""
        try:
            self._ser = serial.Serial(self._port, self._baud, timeout=1)
            time.sleep(2)   # allow Arduino reset after DTR toggle
            logger.info(f"Connected to Arduino on {self._port} at {self._baud} baud.")
            return True
        except serial.SerialException as e:
            logger.error(f"Serial connection failed: {e}")
            return False

    def disconnect(self) -> None:
        """Close the serial port."""
        self._running = False
        if self._ser and self._ser.is_open:
            self._ser.close()
            logger.info("Serial port closed.")

    # ── Send ──────────────────────────────────────────────────
    def send(self, msg: str) -> None:
        """Send a newline-terminated command to the Arduino."""
        if self._ser and self._ser.is_open:
            self._ser.write((msg + "\n").encode("utf-8"))
            logger.debug(f"Sent: {msg}")
            print(f"  [to Arduino] {msg}")

    # ── Receive loop (runs in background thread) ──────────────
    def _read_loop(self) -> None:
        while self._running:
            try:
                if self._ser and self._ser.in_waiting:
                    raw  = self._ser.readline()
                    line = raw.decode("utf-8", errors="ignore").strip()
                    if line:
                        self._dispatch(line)
                else:
                    time.sleep(0.03)
            except serial.SerialException as e:
                logger.error(f"Serial read error: {e}")
                break

    def _dispatch(self, line: str) -> None:
        """Route an incoming Arduino message to the correct handler."""
        # Countdown messages: update same terminal line
        if line.startswith("COUNTDOWN:"):
            print(f"\r  Entry delay: {line[10:]}s remaining    ", end="", flush=True)
            return
        if line.startswith("EXIT_COUNTDOWN:"):
            print(f"\r  Exit now!  {line[15:]}s remaining      ", end="", flush=True)
            return

        print(f"\n  [Arduino] {line}")

        if line == "CMD:FACE_VERIFY":
            threading.Thread(target=self._handle_face_verify, daemon=True).start()

        elif line == "CMD:FACE_ENROL":
            threading.Thread(target=self._handle_face_enrol, daemon=True).start()

        elif line == "CMD:CALL_EMERGENCY":
            self._notifier.call_emergency_service()

        elif line.startswith("CMD:NOTIFY:"):
            self._notifier.send_notification(line[11:])

        elif line.startswith("LOG:"):
            parts = line[4:].split(":", 1)
            event = parts[1] if len(parts) == 2 else line[4:]
            self._log_csv(event)

        elif "ALARM_TRIGGERED" in line:
            print("\n  !! ALARM TRIGGERED !!")

        elif "DISARMED" in line:
            print("\n  System DISARMED - door unlocked")

        elif "ARMED" in line:
            print("\n  System ARMED")

    # ── Face handlers ─────────────────────────────────────────
    def _handle_face_verify(self) -> None:
        matched = self._face.verify(timeout_seconds=10)
        self.send("FACE:MATCH" if matched else "FACE:NO_MATCH")

    def _handle_face_enrol(self) -> None:
        success = self._face.enroll_face(label="authorised_user")
        self.send("FACE:ENROLLED" if success else "STATUS:ENROL_FAILED")

    # ── Lifecycle ─────────────────────────────────────────────
    def start(self) -> None:
        """Start the background receive thread."""
        self._running = True
        self._thread  = threading.Thread(target=self._read_loop, daemon=True)
        self._thread.start()
        logger.info("Serial receive thread started.")

    def stop(self) -> None:
        """Stop the receive thread and close the port."""
        self.disconnect()
        if self._thread:
            self._thread.join(timeout=2)
        logger.info("SerialComm stopped.")
