"""
face_detection.py  -  FaceDetection class
Logitech HD C270 webcam via OpenCV + dlib face_recognition.
"""
import cv2
import face_recognition
import numpy as np
import pickle
import os
import logging
import time
from typing import List

logger  = logging.getLogger(__name__)
DB_PATH = "face_database.pkl"


class FacialFeature:
    """Stores a single enrolled face as a 128-d dlib encoding."""
    def __init__(self, label: str, encoding: np.ndarray):
        self.label    = label
        self.encoding = encoding


class FaceDetection:
    """
    Handles face enrolment and verification.

    Parameters
    ----------
    match_threshold : float  0.0-1.0; lower = stricter  (default 0.5)
    camera_index    : int    OpenCV index for Logitech C270 (default 0;
                             use 1 if laptop has built-in webcam)
    """

    def __init__(self, match_threshold: float = 0.5, camera_index: int = 0):
        self.match_threshold = match_threshold
        self.camera_index    = camera_index
        self.facial_database: List[FacialFeature] = []
        self._load_database()

    # ── Database ──────────────────────────────────────────────
    def _load_database(self) -> None:
        if os.path.exists(DB_PATH):
            with open(DB_PATH, "rb") as f:
                self.facial_database = pickle.load(f)
            logger.info(f"Loaded {len(self.facial_database)} enrolled face(s).")

    def _save_database(self) -> None:
        with open(DB_PATH, "wb") as f:
            pickle.dump(self.facial_database, f)

    # ── Camera ────────────────────────────────────────────────
    def _open_camera(self) -> cv2.VideoCapture:
        """
        Open the Logitech C270 at 640x480 for processing speed.
        The C270 is natively 1280x720 but detection is faster at half res.
        """
        backend = cv2.CAP_DSHOW if os.name == "nt" else cv2.CAP_V4L2
        cap = cv2.VideoCapture(self.camera_index, backend)
        if not cap.isOpened():
            cap = cv2.VideoCapture(self.camera_index)
        if not cap.isOpened():
            raise RuntimeError(
                f"Cannot open Logitech C270 at camera index {self.camera_index}. "
                "Check USB connection."
            )
        cap.set(cv2.CAP_PROP_FRAME_WIDTH,  640)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
        cap.set(cv2.CAP_PROP_FPS, 30)
        return cap

    # ── Verification (called by SerialComm on CMD:FACE_VERIFY) ─
    def verify(self, timeout_seconds: int = 10) -> bool:
        """
        Open camera, detect face, compare to enrolled database.
        Returns True if a registered face is found within the timeout.
        """
        logger.info("Face verification started.")
        cap    = self._open_camera()
        result = False
        start  = time.time()

        while time.time() - start < timeout_seconds:
            ret, frame = cap.read()
            if not ret:
                continue
            small = cv2.resize(frame, (0, 0), fx=0.5, fy=0.5)
            rgb   = cv2.cvtColor(small, cv2.COLOR_BGR2RGB)
            locs  = face_recognition.face_locations(rgb, model="hog")
            if locs:
                encs = face_recognition.face_encodings(rgb, locs)
                if encs:
                    result = self._compare(encs[0])
                    break
            time.sleep(0.05)

        cap.release()
        logger.info(f"Verification result: {'MATCH' if result else 'NO MATCH'}")
        return result

    # ── Enrolment (interactive, needs a display) ──────────────
    def enroll_face(self, label: str = "authorised_user") -> bool:
        """
        Interactive face enrolment using the Logitech C270.
        Opens a preview window. Press SPACE to capture (5 samples), Q to quit.
        Returns True on success (>= 3 samples captured).
        """
        logger.info(f"Starting enrolment for '{label}'.")
        cap, samples = self._open_camera(), []
        print(f"\n[FaceDetection] Enrolling '{label}'")
        print("  Position your face — press SPACE to capture, Q to cancel.\n")

        while len(samples) < 5:
            ret, frame = cap.read()
            if not ret:
                break
            rgb  = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            locs = face_recognition.face_locations(rgb, model="hog")

            for top, right, bottom, left in locs:
                cv2.rectangle(frame, (left, top), (right, bottom), (0, 200, 80), 2)

            cv2.putText(frame,
                        f"Sample {len(samples)}/5  |  SPACE=capture  Q=quit",
                        (10, 28), cv2.FONT_HERSHEY_SIMPLEX, 0.65, (0, 200, 80), 2)
            cv2.imshow(f"Enrol: {label}", frame)

            key = cv2.waitKey(1) & 0xFF
            if key == ord(" ") and locs:
                encs = face_recognition.face_encodings(rgb, locs)
                if encs:
                    samples.append(encs[0])
                    print(f"  Sample {len(samples)} captured.")
            elif key == ord("q"):
                print("  Enrolment cancelled.")
                break

        cap.release()
        cv2.destroyAllWindows()

        if len(samples) >= 3:
            avg = np.mean(samples, axis=0)
            self.facial_database = [f for f in self.facial_database
                                      if f.label != label]
            self.facial_database.append(FacialFeature(label, avg))
            self._save_database()
            logger.info(f"Enrolled '{label}' with {len(samples)} samples.")
            return True

        logger.warning("Enrolment failed: not enough samples.")
        return False

    # ── Internal comparison ───────────────────────────────────
    def _compare(self, encoding: np.ndarray) -> bool:
        if not self.facial_database:
            logger.warning("Face database is empty. Enrol a face first.")
            return False
        known = [f.encoding for f in self.facial_database]
        dists = face_recognition.face_distance(known, encoding)
        idx   = int(np.argmin(dists))
        if dists[idx] <= self.match_threshold:
            logger.info(f"Match: '{self.facial_database[idx].label}'  dist={dists[idx]:.3f}")
            return True
        logger.info(f"No match. Best dist={dists[idx]:.3f}")
        return False
