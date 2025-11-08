<?php
// Pad naar je Maildir
$maildir = "./";

/**
 * Controleer of een bestand een zstd-bestand is via magic bytes
 */
function is_zstd($filePath) {
    $fp = fopen($filePath, "rb");
    if (!$fp) return false;
    $magic = fread($fp, 4);
    fclose($fp);
    return $magic === "\x28\xB5\x2F\xFD";
}

/**
 * Decomprimeer een zstd-bestand en overschrijf het originele bestand
 */
function decompress_file($filePath) {
    // -f = force overwrite
    $command = "zstd -d -f " . escapeshellarg($filePath);
    exec($command, $output, $returnVar);

    if ($returnVar === 0) {
        echo "[OK] Uitgepakt: $filePath\n";
    } else {
        echo "[FOUT] $filePath kon niet uitgepakt worden\n";
    }
}

/**
 * Doorloop alle 'cur' submappen en decompress bestanden
 */
function process_maildir_cur($maildir) {
    $dirIterator = new RecursiveIteratorIterator(
        new RecursiveDirectoryIterator($maildir, RecursiveDirectoryIterator::SKIP_DOTS),
        RecursiveIteratorIterator::SELF_FIRST
    );

    foreach ($dirIterator as $file) {
        if ($file->isFile() && strtolower($file->getPathInfo()->getFilename()) === 'cur') {
            // 'cur' is een map, niet een bestand; overslaan
            continue;
        }

        // Check of het in een 'cur' map staat
        if (basename($file->getPath()) === 'cur') {
            $filePath = $file->getPathname();
            if (!is_zstd($filePath)) {
                echo "[SKIP] Geen zstd-bestand: $filePath\n";
                continue;
            }
            decompress_file($filePath);
        }
    }
}

process_maildir_cur($maildir);

