"""
notification_service.py  -  NotificationService class
Sends email alerts via SMTP when the alarm triggers.
Configure using environment variables (see README).
"""
import smtplib
import logging
import os
import datetime
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from typing import List

logger = logging.getLogger(__name__)


class NotificationService:
    """
    Sends email alerts and logs notifications in memory.

    Environment variables required for email:
        SMTP_HOST   (default smtp.gmail.com)
        SMTP_PORT   (default 587)
        SMTP_USER   sending address
        SMTP_PASS   app password (Gmail: use App Password, not account password)
        ALERT_EMAIL destination address
    """

    def __init__(self):
        self._notification_log: List[str] = []
        self._host = os.getenv("SMTP_HOST",  "smtp.gmail.com")
        self._port = int(os.getenv("SMTP_PORT", "587"))
        self._user = os.getenv("SMTP_USER",  "")
        self._pw   = os.getenv("SMTP_PASS",  "")
        self._dest = os.getenv("ALERT_EMAIL", "")

    def send_notification(self, message: str) -> None:
        """Log the notification and send an email if SMTP is configured."""
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        entry     = f"[{timestamp}] {message}"
        self._notification_log.append(entry)
        logger.info(entry)
        print(f"\n  [ALERT] {entry}")

        if not self._user or not self._dest:
            logger.warning("SMTP not configured — alert printed to console only.")
            return

        try:
            msg             = MIMEMultipart()
            msg["From"]     = self._user
            msg["To"]       = self._dest
            msg["Subject"]  = "BURGLAR ALARM ALERT"
            body            = f"BURGLAR ALARM NOTIFICATION\n\n{message}\n\nTimestamp: {timestamp}"
            msg.attach(MIMEText(body, "plain"))

            with smtplib.SMTP(self._host, self._port) as server:
                server.starttls()
                server.login(self._user, self._pw)
                server.send_message(msg)

            logger.info("Email alert sent successfully.")
        except Exception as e:
            logger.error(f"Failed to send email: {e}")

    def call_emergency_service(self) -> None:
        """Notify emergency services (currently via email + console)."""
        self.send_notification(
            "EMERGENCY: Burglar alarm has been triggered. "
            "Emergency services have been notified. Please check the premises."
        )

    def get_log(self) -> List[str]:
        """Return a copy of the in-memory notification log."""
        return self._notification_log.copy()

    def save_log_to_file(self, path: str = "notification_log.txt") -> None:
        """Persist the notification log to a text file."""
        with open(path, "w") as f:
            for entry in self._notification_log:
                f.write(entry + "\n")
