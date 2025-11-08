<?php

$maildir = './';
$c=0;
$e=0;

/**
 * Controleer of een bestand een zstd-bestand is via magic bytes
 */
function is_zstd($filePath) : bool
{
    $fp = fopen($filePath, "rb");
    if (!$fp) return false;
    $magic = fread($fp, 4);
    fclose($fp);
    return $magic === "\x28\xB5\x2F\xFD";
}



/**
 * Functie die wordt uitgevoerd voor elk gevonden bestand.
 * Hier voer je je specifieke logica uit (bijv. lezen, schrijven, zstd decompressie, etc.).
 *
 * @param SplFileInfo $file Het bestandsobject met alle informatie over het bestand.
 */
/**
 * Decomprimeer een zstd-bestand en overschrijf het originele bestand
 */
function decompress_file($filePath) : void
{
    global $c,$e;
    $src = escapeshellarg($filePath);
    $tmp = $src . ".tmp";
    $command = "zstd -d " . $src . " -o " . $tmp;
    exec($command, $output, $returnVar);

    if ($returnVar === 0) {
        echo "[OK]";
        $c++;
        exec("mv -f {$tmp} {$src}", $output, $returnVar);
    } else {
        echo "[FOUT]: {$filePath}\n";
        $e++;
        exec("rm -f {$tmp}", $output, $returnVar);
    }
}

// --- De Recursieve Loop ---

try {
    // 1. Maak een RecursiveDirectoryIterator: dit gaat de directory structuur in.
    $iterator = new RecursiveDirectoryIterator($maildir);
    
    // 2. Maak een RecursiveIteratorIterator: dit zorgt ervoor dat de loop
    //    ALLE bestanden en submappen doorloopt.
    $files = new RecursiveIteratorIterator($iterator);
    
    echo "Start recursieve zoektocht in: {$maildir}\n";
    echo str_repeat('-', 40) . "\n";

    // Loop over elk item (bestand of map) dat de iterator vindt
    foreach ($files as $file) {
        
        if ($file->isDir()) { continue; }
        
        $f = $file->getPathname();
        
        // Controleer of het een ZSTD bestand is
        if (!is_zstd($f)) {
            echo "[SKIP] Geen zstd bestand: {$f}\n";
            continue;
        }
        
        decompress_file($f);
    }
    
    echo str_repeat('-', 40) . "\n";
    echo "   {$c} files successfully processed.\n";
    echo "   {$e} decompression errors encountered.\n\n";

} catch (UnexpectedValueException $e) {
    echo "[ERROR] Cannot open {$maildir}\n";
    echo "Message: " . $e->getMessage() . "\n";
}

?>

