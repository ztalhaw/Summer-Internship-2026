# gguf_dump — Header & Metadata Parser (Talha's part)

## What this is

A small C program that opens a `.gguf` model file (the format used by
llama.cpp / Ollama to store an LLM's config, tokenizer, and weights in a
single file) and prints out its header and all of its metadata key-value
pairs in a human-readable way.

This is my (Talha's) piece of a 3-person team split:

| Member | Part |
|---|---|
| **Talha (me)** | Header + metadata + CLI entry point |
| Member 1 | Tensor table + alignment/offset math |
| Member 2 | Dequantization (Q8_0 / Q4_0) |

## How to build and run

```bash
gcc gguf_dump.c -o gguf_dump
./gguf_dump model.gguf
```

If you forget the filename, the program prints a usage message instead of
crashing:

```bash
./gguf_dump
# usage: ./gguf_dump <path_to_model.gguf>
```

## File layout (what I'm parsing)

A GGUF file is three sections back to back. My part only covers the first
two — Member 1 picks up from where the tensor table starts.

```
0                                                        file_size
├───────────────┬─────────────────────────┬────────────┬─────────┤
│    HEADER     │        METADATA         │  TENSOR    │  TENSOR │
│  magic "GGUF" │   key-value pairs:       │  TABLE     │  DATA   │
│  version      │   "llama.block_count"=22 │ (Member 1) │ (padded │
│  tensor_count │   "tokenizer.ggml.model" │            │  to 32) │
│  metadata_cnt │   = "llama", ...arrays   │            │         │
└───────────────┴─────────────────────────┴────────────┴─────────┘
 ^ my part starts here    ^ my part ends here
```

Each metadata entry looks like:

```
[ key: length-prefixed string ][ type_id: 4 bytes ][ value: type_id bytes ]
```

`type_id` tells the parser what the value actually is (int, float, string,
array, ...) — the numbering is fixed by the GGUF spec:

```
0  UINT8    4  UINT32   8  STRING   11 INT64
1  INT8     5  INT32    9  ARRAY    12 FLOAT64
2  UINT16   6  FLOAT32
3  INT16    7  BOOL     10 UINT64
```

## Steps I took

1. **Opened the file with `open()` + `fstat()`** to get a file descriptor
   and the file's size in bytes.
2. **Memory-mapped the whole file with `mmap()`** (`PROT_READ`,
   `MAP_SHARED`, offset `0`) instead of `read()`-ing it in chunks, so I
   could just walk through it with a raw `uint8_t *` pointer and treat any
   chunk of bytes as whatever type I needed via pointer casts.
3. **Checked the magic number.** The first 4 bytes must equal the ASCII
   letters `GGUF`. If they don't, the file isn't a valid GGUF file and I
   bail out early (after `munmap`-ing and `close`-ing cleanly).
4. **Read the fixed-size header fields** right after the magic number, in
   order: `version` (4 bytes), `tensor_count` (8 bytes), `metadata_count`
   (8 bytes) — each one read via a pointer cast, then the pointer advanced
   past it.
5. **Wrote a `readstring()` helper.** GGUF strings aren't null-terminated
   like C strings — they're stored as an 8-byte length followed by raw
   bytes. This helper reads the length, `memcpy`s that many bytes into a
   local buffer, and manually appends `'\0'` so the rest of the program
   can treat it as a normal C string. It returns the pointer advanced past
   the string so the caller stays in sync with the file position.
6. **Looped over `metadata_count` entries.** For each one: read the key
   name (always a string), read the 4-byte `type_id`, then `switch` on
   `type_id` to read and print the value with the right size/format.
7. **Handled the `ARRAY` type (type 9) as a special case** — arrays store
   an item type + item count up front, then that many raw values packed
   back to back with no per-item type tags. I don't print every array
   element individually (not required for the assignment); I just skip
   over the correct number of bytes per element so the main pointer ends
   up in the right place for whatever comes next in the file.
8. **Cleaned up with `munmap()` and `close()`** at every exit path,
   including the error paths, so nothing leaks even when the program
   bails out early.

## Bugs I hit and fixed along the way

- **Array-skipping only advanced 1 byte for 2-byte types.** My first pass
  at the `ARRAY` case grouped `UINT16`/`INT16` into the same "else, skip 1
  byte" branch as the 1-byte types. That silently desynced the parser for
  every metadata entry that came after a 16-bit array — the rest of the
  file would be read from the wrong offset. Fixed by giving
  `UINT16`/`INT16` (item types 2 and 3) their own branch that advances the
  pointer by 2 bytes.
- **`FLOAT64` (type 12) wasn't handled** in the main `switch` at first —
  any file with a double-valued metadata key would fall into `default:`
  and abort with "unknown type id", even though the file was completely
  valid. Fixed by adding a `case 12` that reads 8 bytes as a `double`.

## Status

- [x] Header parses correctly: magic check, version, tensor count,
      metadata count
- [x] All metadata key-value types print correctly, including arrays
      (skipped correctly, not misaligning the rest of the file) and
      FLOAT64
- [ ] Tensor-by-name lookup — depends on Member 1's tensor table code
- [ ] Q8_0 dequantized values matching Python's `gguf` library — depends
      on Member 2's part
