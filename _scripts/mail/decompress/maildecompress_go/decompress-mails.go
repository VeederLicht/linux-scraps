// decompress_maildir.go
package main

import (
	"bytes"
	"fmt"
	"io"
	"io/fs"
	"os"
	"path/filepath"

	"github.com/klauspost/compress/zstd"
)

var zstdMagic = []byte{0x28, 0xB5, 0x2F, 0xFD}

func isZstdFile(path string) (bool, error) {
	f, err := os.Open(path)
	if err != nil {
		return false, err
	}
	defer f.Close()

	buf := make([]byte, 4)
	n, err := f.Read(buf)
	if err != nil && err != io.EOF {
		return false, err
	}
	if n < 4 {
		return false, nil
	}
	return bytes.Equal(buf, zstdMagic), nil
}

func decompressStreamToTemp(srcPath string, info fs.FileInfo) error {
	// Open source for reading
	src, err := os.Open(srcPath)
	if err != nil {
		return fmt.Errorf("open source: %w", err)
	}
	defer src.Close()

	// Create temp file in same directory
	dir := filepath.Dir(srcPath)
	tmpFile, err := os.CreateTemp(dir, ".decompress-*.tmp")
	if err != nil {
		return fmt.Errorf("create temp: %w", err)
	}
	tmpPath := tmpFile.Name()

	// Ensure temp file closed & removed on error
	defer func() {
		tmpFile.Close()
	}()

	// Create zstd reader (streaming)
	dec, err := zstd.NewReader(src)
	if err != nil {
		// remove temp file
		_ = tmpFile.Close()
		_ = os.Remove(tmpPath)
		return fmt.Errorf("create zstd reader: %w", err)
	}
	defer dec.Close()

	// Copy stream to temp file
	if _, err := io.Copy(tmpFile, dec); err != nil {
		_ = tmpFile.Close()
		_ = os.Remove(tmpPath)
		return fmt.Errorf("stream copy: %w", err)
	}

	// Close files to flush
	if err := tmpFile.Close(); err != nil {
		_ = os.Remove(tmpPath)
		return fmt.Errorf("close temp: %w", err)
	}
	if err := src.Close(); err != nil {
		// best-effort continue
	}

	// Preserve original permissions
	if err := os.Chmod(tmpPath, info.Mode().Perm()); err != nil {
		// ignore failure to chmod but warn
		fmt.Fprintf(os.Stderr, "warning: chmod failed for %s: %v\n", tmpPath, err)
	}

	// Atomically replace original file
	if err := os.Rename(tmpPath, srcPath); err != nil {
		_ = os.Remove(tmpPath)
		return fmt.Errorf("rename tmp to src: %w", err)
	}

	return nil
}

func processAllFiles(root string) error {
	return filepath.WalkDir(root, func(path string, d fs.DirEntry, walkErr error) error {
		if walkErr != nil {
			// If WalkDir encountered an error for this path, propagate it
			return walkErr
		}

		// only process regular files
		if !d.Type().IsRegular() {
			return nil
		}

		// Optional: skip common temporary/backup files (.tmp .bak). Adjust as needed
		base := filepath.Base(path)
		if len(base) > 0 && base[0] == '.' {
			// skip hidden files like .decompress-abc.tmp
			return nil
		}

		// Quick check: is it Zstd?
		isZ, err := isZstdFile(path)
		if err != nil {
			fmt.Fprintf(os.Stderr, "[ERROR] check zstd for %s: %v\n", path, err)
			return nil // continue walking other files
		}
		if !isZ {
			fmt.Printf("[SKIP] %s (not zstd)\n", path)
			return nil
		}

		// get FileInfo for permissions
		info, err := d.Info()
		if err != nil {
			fmt.Fprintf(os.Stderr, "[ERROR] stat %s: %v\n", path, err)
			return nil
		}

		// Decompress streaming and overwrite original
		if err := decompressStreamToTemp(path, info); err != nil {
			fmt.Fprintf(os.Stderr, "[FOUT] %s: %v\n", path, err)
		} else {
			fmt.Printf("[OK] %s\n", path)
		}

		return nil
	})
}

func main() {
	if len(os.Args) != 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s /pad/naar/Maildir\n", os.Args[0])
		os.Exit(2)
	}
	root := os.Args[1]

	// Basic sanity check
	info, err := os.Stat(root)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Invalid root path: %v\n", err)
		os.Exit(1)
	}
	if !info.IsDir() {
		fmt.Fprintf(os.Stderr, "Root is not a directory\n")
		os.Exit(1)
	}

	if err := processAllFiles(root); err != nil {
		fmt.Fprintf(os.Stderr, "Processing error: %v\n", err)
		os.Exit(1)
	}
}

