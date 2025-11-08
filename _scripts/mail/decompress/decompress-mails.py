# SCRIPT FOR DECOMPRESSING (ZSTD) MAILS IN MAILDIR FOLDER (ChatGPT)

import os
import zstandard as zstd
import tempfile
import shutil

# Pas dit aan naar jouw Maildir-map
MAILDIR = "./"

def is_zstd(file_path):
    """Controleer magic bytes van een zstd-bestand"""
    try:
        with open(file_path, "rb") as f:
            magic = f.read(4)
        return magic == b'\x28\xb5\x2f\xfd'
    except Exception:
        return False

def decompress_file_stream(file_path):
    """
    Streaming decompressie van een zstd-bestand.
    Het originele bestand wordt veilig overschreven via een tijdelijke file.
    """
    tmp_fd, tmp_path = tempfile.mkstemp(dir=os.path.dirname(file_path))
    os.close(tmp_fd)  # we gebruiken open() later

    try:
        with open(file_path, 'rb') as fin, open(tmp_path, 'wb') as fout:
            dctx = zstd.ZstdDecompressor()
            dctx.copy_stream(fin, fout)
        # Vervang origineel bestand door de decompressie
        shutil.move(tmp_path, file_path)
        print(f"[OK] Uitgepakt: {file_path}")
    except Exception as e:
        print(f"[FOUT] {file_path} kon niet uitgepakt worden: {e}")
        if os.path.exists(tmp_path):
            os.remove(tmp_path)

def process_maildir_cur(maildir_path):
    """
    Doorloop alle 'cur' submappen van de Maildir en decompress bestanden.
    """
    for root, dirs, files in os.walk(maildir_path):
        if os.path.basename(root) == 'cur':
            for filename in files:
                file_path = os.path.join(root, filename)
                if not is_zstd(file_path):
                    print(f"[SKIP] Geen zstd-bestand: {file_path}")
                    continue
                decompress_file_stream(file_path)

if __name__ == "__main__":
    process_maildir_cur(MAILDIR)

