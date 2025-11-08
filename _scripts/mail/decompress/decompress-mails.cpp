#include <cstring>   // voor memcmp
#include <iostream>
#include <fstream>
#include <filesystem>
#include <zstd.h>
#include <vector>
#include <cstdio>

namespace fs = std::filesystem;

const unsigned char ZSTD_MAGIC[4] = {0x28, 0xB5, 0x2F, 0xFD};

bool is_zstd(const fs::path& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) return false;

    unsigned char magic[4];
    file.read(reinterpret_cast<char*>(magic), 4);
    file.close();

    return memcmp(magic, ZSTD_MAGIC, 4) == 0;
}

bool decompress_file_stream(const fs::path& filepath) {
    // Tijdelijke file
    fs::path tmp_path = filepath;
    tmp_path += ".tmp";

    std::ifstream fin(filepath, std::ios::binary);
    if (!fin) {
        std::cerr << "[FOUT] Kan bestand niet openen: " << filepath << std::endl;
        return false;
    }

    std::ofstream fout(tmp_path, std::ios::binary);
    if (!fout) {
        std::cerr << "[FOUT] Kan tijdelijke file niet openen: " << tmp_path << std::endl;
        return false;
    }

    ZSTD_DStream* dstream = ZSTD_createDStream();
    if (!dstream) {
        std::cerr << "[FOUT] Kan ZSTD stream niet aanmaken\n";
        return false;
    }
    if (ZSTD_isError(ZSTD_initDStream(dstream))) {
        std::cerr << "[FOUT] Kan ZSTD stream niet initialiseren\n";
        ZSTD_freeDStream(dstream);
        return false;
    }

    size_t inSize = ZSTD_DStreamInSize();
    size_t outSize = ZSTD_DStreamOutSize();
    std::vector<char> inBuffer(inSize);
    std::vector<char> outBuffer(outSize);

    while (fin) {
        fin.read(inBuffer.data(), inSize);
        std::streamsize readBytes = fin.gcount();
        if (readBytes <= 0) break;

        ZSTD_inBuffer input = { inBuffer.data(), static_cast<size_t>(readBytes), 0 };

        while (input.pos < input.size) {
            ZSTD_outBuffer output = { outBuffer.data(), outSize, 0 };
            size_t ret = ZSTD_decompressStream(dstream, &output, &input);
            if (ZSTD_isError(ret)) {
                std::cerr << "[FOUT] " << filepath << " kon niet uitgepakt worden: "
                          << ZSTD_getErrorName(ret) << std::endl;
                ZSTD_freeDStream(dstream);
                fin.close();
                fout.close();
                fs::remove(tmp_path);
                return false;
            }
            if (output.pos > 0) {
                fout.write(outBuffer.data(), output.pos);
            }
        }
    }

    ZSTD_freeDStream(dstream);
    fin.close();
    fout.close();

    // Overschrijf origineel
    std::error_code ec;
    fs::rename(tmp_path, filepath, ec);
    if (ec) {
        std::cerr << "[FOUT] Kan origineel bestand niet vervangen: " << filepath << std::endl;
        fs::remove(tmp_path);
        return false;
    }

    std::cout << "[OK] Uitgepakt: " << filepath << std::endl;
    return true;
}

void process_maildir_cur(const fs::path& path) {
    for (auto& entry : fs::directory_iterator(path)) {
        if (fs::is_directory(entry)) {
            if (entry.path().filename() == "cur") {
                for (auto& mail : fs::directory_iterator(entry.path())) {
                    if (!fs::is_regular_file(mail)) continue;
                    const fs::path& mail_path = mail.path();

                    if (!is_zstd(mail_path)) {
                        std::cout << "[SKIP] Geen zstd-bestand: " << mail_path << std::endl;
                        continue;
                    }

                    decompress_file_stream(mail_path);
                }
            } else {
                process_maildir_cur(entry.path());
            }
        }
    }
}

int main() {
    fs::path maildir = "./"; // pas dit aan
    if (!fs::exists(maildir) || !fs::is_directory(maildir)) {
        std::cerr << "Maildir bestaat niet: " << maildir << std::endl;
        return 1;
    }

    process_maildir_cur(maildir);
    return 0;
}

