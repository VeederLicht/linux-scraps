#!/usr/bin/env luajit

local ffi = require("ffi")
local zstd = ffi.load("zstd")

-- Zstandard C API
ffi.cdef[[

typedef struct {
    void* src;
    size_t size;
    size_t pos;
} ZSTD_inBuffer;

typedef struct {
    void* dst;
    size_t size;
    size_t pos;
} ZSTD_outBuffer;

typedef struct ZSTD_DStream_s ZSTD_DStream;

ZSTD_DStream* ZSTD_createDStream(void);
size_t ZSTD_freeDStream(ZSTD_DStream* zds);

size_t ZSTD_initDStream(ZSTD_DStream* zds);

size_t ZSTD_decompressStream(ZSTD_DStream* zds,
                             ZSTD_outBuffer* output,
                             ZSTD_inBuffer* input);

unsigned ZSTD_isError(size_t code);
const char* ZSTD_getErrorName(size_t code);

]]


--------------------------------------------------------------------
-- Hulpfunctie: check Zstd magic bytes
--------------------------------------------------------------------
local function is_zstd_file(path)
    local f = io.open(path, "rb")
    if not f then return false end
    local magic = f:read(4)
    f:close()
    return magic == "\x28\xB5\x2F\xFD"
end

--------------------------------------------------------------------
-- Streaming decompressie via libzstd
--------------------------------------------------------------------
local function decompress_file(path)
    local in_f = assert(io.open(path, "rb"))
    local tmp_path = path .. ".tmp"
    local out_f = assert(io.open(tmp_path, "wb"))

    -- Maak streaming context
    local dstream = zstd.ZSTD_createDStream()
    if dstream == nil then
        error("Kon ZSTD DStream niet maken")
    end

    local init = zstd.ZSTD_initDStream(dstream)
    if zstd.ZSTD_isError(init) ~= 0 then
        error("ZSTD_initDStream error: " .. ffi.string(zstd.ZSTD_getErrorName(init)))
    end

    -- Buffers
    local in_buf_size = 16384
    local out_buf_size = 16384

    local in_buf_mem = ffi.new("uint8_t[?]", in_buf_size)
    local out_buf_mem = ffi.new("uint8_t[?]", out_buf_size)

    local inBuffer = ffi.new("ZSTD_inBuffer")
    local outBuffer = ffi.new("ZSTD_outBuffer")

    -- Streaming loop
    while true do
        local chunk = in_f:read(in_buf_size)
        if not chunk then break end
        ffi.copy(in_buf_mem, chunk, #chunk)

        inBuffer.src = in_buf_mem
        inBuffer.size = #chunk
        inBuffer.pos = 0

        while inBuffer.pos < inBuffer.size do
            outBuffer.dst = out_buf_mem
            outBuffer.size = out_buf_size
            outBuffer.pos = 0

            local ret = zstd.ZSTD_decompressStream(dstream, outBuffer, inBuffer)
            if zstd.ZSTD_isError(ret) ~= 0 then
                out_f:close()
                in_f:close()
                os.remove(tmp_path)
                error(string.format("[FOUT] %s: %s",
                    path,
                    ffi.string(zstd.ZSTD_getErrorName(ret))
                ))
            end

            if outBuffer.pos > 0 then
                out_f:write(ffi.string(out_buf_mem, outBuffer.pos))
            end
        end
    end

    out_f:close()
    in_f:close()

    zstd.ZSTD_freeDStream(dstream)

    -- Veilig vervangen
    os.remove(path)
    os.rename(tmp_path, path)
    print("[OK] Uitgepakt: " .. path)
end

--------------------------------------------------------------------
-- Doorloop alle Maildir `cur` mappen
--------------------------------------------------------------------
local maildir = "./"

local function process_maildir(root)
    local cmd = 'find "' .. root .. '" -type d -name cur'
    local pipe = io.popen(cmd, "r")

    for cur_path in pipe:lines() do
        local list = io.popen('find "'..cur_path..'" -maxdepth 1 -type f', "r")
        for file in list:lines() do
            if is_zstd_file(file) then
                decompress_file(file)
            else
                print("[SKIP] Geen zstd-bestand: " .. file)
            end
        end
        list:close()
    end

    pipe:close()
end

process_maildir(maildir)

