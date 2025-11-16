#!/usr/bin/env python3
import os
import sys
import email
from datetime import datetime, timezone
from email.utils import parsedate_to_datetime, parseaddr
import logging
import socket
import random

logging.basicConfig(
    filename="rename-mails-py.log",
    level=logging.INFO,
    format="%(asctime)s  %(message)s",
    filemode="w"
)

n_success = 0
n_failure = 0
n_skipped = 0

# ####################################################################
# Processess a single email file to extract 'From' and 'Date' headers
def process_email_file(filepath, mode):
    global n_success, n_failure, n_skipped
    try:
        with open(filepath, "rb") as f:
            msg = email.message_from_binary_file(f)
    except Exception:
        n_skipped += 1
        logging.info(f"[SKIP]  no valid MIME file:{filepath}")
        return  # geen geldig MIME bericht

    from_header = msg.get("From")
    date_header = msg.get("Date")

    if not from_header or not date_header:
        n_failure += 1
        logging.info(f"[ERROR]  MIME fields missing: {filepath}")
        return

    # Datum → Unix timestamp (UTC)
    try:
        # Emailadres isoleren
        _, email_addr = parseaddr(from_header)

        dt = parsedate_to_datetime(date_header)

        # Zorg dat de datetime timezone-aware is
        if dt.tzinfo is None:
            dt = dt.replace(tzinfo=timezone.utc)

        # Converteer naar UTC
        dt_utc = dt.astimezone(timezone.utc)

    except Exception:
        n_failure += 1
        logging.info(f"[ERROR]  cannot parse fields: {filepath}")
        return

    # Unix timestamp
    timestamp = int(dt_utc.timestamp())

    # Select handler based on mode
    match mode:
        case "dryrun":
            n_success += 1
            logging.info(f"[OK]  dryrun on file: {filepath} -> '{email_addr}' / '{timestamp}'")
            return
        case "natural":
            # Natuurlijke sorteervolgorde: YYYYMMDD-HHMMSS
            dt_str = dt_utc.strftime("%Y%m%d-%H%M%S")
            new_filename = f"{dt_str}--[FROM]_{email_addr}.eml"
        case "maildir":
            # Maildir formaat: timestamp.MillisecondsPadded-UniqueID.Hostname
            size_S= os.path.getsize(filepath)
            unique_id = f"M{random.randint(100000,999999)}P{random.randint(100000,999999)}"
            hostname = socket.gethostname()
            new_filename = f"{timestamp}.{unique_id}.{hostname},S={size_S},W={size_S}:2,S"
        case _:
            logging.info(f"[ERROR]  unknown mode: {mode}")
            sys.exit(1)
        
    try:
        new_filepath = os.path.join(root, new_filename)
        os.rename(filepath, new_filepath)
    except Exception:
        n_failure += 1
        logging.info(f"[ERROR]  cannot rename file: {filepath}")
        return

    n_success += 1
    logging.info(f"[OK]  renamed: {filepath} -> {new_filename}")




if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Use: {sys.argv[0]} <mode> <startdir>")
        sys.exit(1)

    modes = ["maildir", "dryrun", "natural"]
    if sys.argv[1] not in modes:
        print(f"Unknown mode: {sys.argv[1]}, choose from {modes}")
        sys.exit(1)

    print("\n\n --------------------------  RENAME EMAILS  -------------------------- \n\n")
    logging.info("--------------------------------- Start renaming emails")

    for root, dirs, files in os.walk(sys.argv[2]):
        for name in files:
            filepath = os.path.join(root, name)
            process_email_file(filepath, sys.argv[1])
    
    logging.info(f"-------------------------- Renaming complete: {n_success} success, {n_failure} failure, {n_skipped} skipped")
    print(f"    Renaming complete: {n_success} success, {n_failure} failure, {n_skipped} skipped\n\n")



