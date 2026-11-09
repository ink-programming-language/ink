# Ink object binary format

This format serializes the logical contents of `ink::vm::ObjectFile`. Its default object format version is `2.0.0`, and its default instruction-set version is `1.1.0`; the supported versions are read through `ConfigManager`.

Object format `2.0.0` removes the reserved BP/SP/PC base-reference entries and the optional-symbol flag from each base-reference record. This is a breaking container-format change. Readers using the defaults reject objects declaring format `1.0.0`; no legacy decoder or automatic migration is provided. Overriding the configured version does not restore the old record encoding. Instruction-set version `1.1.0` adds the EFLAGS register, comparisons, branches and return instruction while retaining existing opcode encodings; see [VM instruction encoding](VM-Instruction-Format.md). Header validation requires the configured instruction-set version to match exactly. Instruction codecs are not yet implemented.

Fields appear in the order below. There is no implicit padding, native pointer, C++ object layout, or terminating byte after a string. All sizes and offsets are byte counts. Section alignment describes loaded storage and does not insert padding into the file.

`u8`, `u16`, `u32`, and `u64` denote unsigned integers of exactly that many bits. The byte-order field controls every multibyte integer in the container. Code, constants, global initializers, and strings are raw byte sequences and are copied verbatim. Code must already contain instructions encoded in the declared byte order; these APIs do not encode individual instructions.

The in-memory `ObjectSymbol::Name` is a `core::Name` handle into the process-wide `core::NamePool`, with `core::NAME_None` as its invalid default. Copies share the pool's immutable spelling while owning independent symbol records. The writer obtains the spelling from the pool and serializes its byte length and bytes; the reader interns those bytes into the current process's pool. Process-local Name indices are never written to the object file. This change to the in-memory name representation preserves the existing wire encoding and object format version `2.0.0`.

## Header

The header occupies 31 bytes. The byte-order field precedes all multibyte fields so readers can interpret the rest without assuming the host byte order.

| Field | Encoding |
| --- | --- |
| Magic | Four bytes: `49 4e 4b 4f` (`INKO`) |
| Byte order | `u8`: `0` for little endian, `1` for big endian |
| Address bit width | `u16`, currently `64` |
| Object format version | Three `u32` values: major, minor, patch |
| Instruction-set version | Three `u32` values: major, minor, patch |

## Code

| Field | Encoding |
| --- | --- |
| Code byte count | `u64` |
| Code | That many raw bytes |

## Constants

| Field | Encoding |
| --- | --- |
| Alignment | `u64`, a nonzero power of two |
| Constant byte count | `u64` |
| Constants | That many raw bytes |

## Global storage

| Field | Encoding |
| --- | --- |
| Alignment | `u64`, a nonzero power of two |
| Total storage byte count | `u64` |
| Initial byte count | `u64`, at most the total storage byte count |
| Initial bytes | That many raw bytes |

Initial bytes describe the beginning of the writable storage. The remaining storage is zero initialized when loaded. Only the initial bytes occupy file space; reading an object does not allocate its declared runtime storage.

## Symbols

A `u64` symbol count is followed by that many records:

| Field | Encoding |
| --- | --- |
| Name byte count | `u64` |
| Name | That many raw string bytes |
| Linkage | `u8`: local `0`, export `1`, import `2` |
| Definition present | `u8`: absent `0`, present `1` |
| Section, if a definition is present | `u8`: code `0`, constants `1`, global storage `2` |
| Byte offset, if a definition is present | `u64`, relative to the selected section |
| Byte size, if a definition is present | `u64` |

Local and exported symbols require definitions. Imported symbols have none. Symbol names must have nonempty spellings registered in the NamePool; invalid handles and empty names are rejected. Name bytes may contain embedded NUL bytes and are preserved by length, without C-string truncation. Import and export names share the object's external namespace.

## Base references

A `u64` entry count is followed by that many records, each exactly four bytes:

| Field | Encoding |
| --- | --- |
| Symbol index | `u32`, a required valid index into the object's symbol table |

The table is empty by default. Its entry indices start at `0` and map only to object-local symbols, including unresolved imports. There are no reserved register entries or per-record presence flags. Counts of index-addressable tables must fit the model's 32-bit indices.

Registers use the independent `RegisterId` numbering space: BP is `0`, SP is `1`, and PC is `2`. A base-reference index of `0` denotes the first symbol reference, not BP. [VM instruction encoding](VM-Instruction-Format.md) distinguishes register-relative addresses from symbol-relative addresses with a compact base prefix; the symbol-reference index carried by an instruction is an index into this base-reference table, whose entry then identifies a symbol.

## Debug information

A `u8` presence flag (`0` or `1`) follows the base references. An absent debug record ends the object. A present record contains:

1. A `u64` source-file count, followed by each source name as a `u64` byte count and that many raw string bytes.
2. A `u64` line-record count, followed by records with `u64` code offset, `u32` file index, `u32` line, and `u32` column, in that order.

Code offsets address the whole code buffer. Source-file indices are object local; line and column numbers start at one. A present but empty debug record is distinct from an absent record.

## Structural validation

`ObjectFileValidator::validate(const ObjectFile &, const core::DiagnosticEngine &) noexcept` returns `void` and requires a diagnostic engine. Success returns normally. Any structural or semantic validation failure reports `VMObjectValidationFailed` through `DiagnosticEngine::reportICE`, with stable diagnostic code `INK-V0003`, number `0x04000003`, and string arguments `Context` and `Description`. The engine prints the diagnostic and terminates even without a consumer, independently of assertions and build mode.

Validation checks the header, section metadata, symbol definitions and identities, base references, and debug records. Unresolved imports are valid object contents. Code bytes remain opaque: validation does not decode instructions or establish legal instruction boundaries, call contracts, or executable behavior.

## Reading and writing

`ObjectFileWriter::write(Object, Diagnostics)` returns the encoded bytes. `writeFile(Object, Path, Diagnostics)` validates and encodes the object before opening the destination in binary truncation mode. Both require the diagnostic engine used for structural validation.

`ObjectFileReader::read(Bytes, Diagnostics)` returns an owned, validated object from a byte span. `readFile(Path, Diagnostics)` reads a binary file and applies the same decoder and validator. A buffer or file contains exactly one object.

Byte decoding and file I/O failures return through `std::expected` and `ObjectFileIOError`, whose fields are `Kind` and `ByteOffset`. Decoder errors include `UnexpectedEnd`, `InvalidEncoding`, `SizeOverflow`, and `TrailingData`; file operations report open, read, write, and close failures explicitly. Lengths and counts are checked before allocating their contents. Structural failures use the fatal validation diagnostic, including invalid decoded objects originating in a byte buffer or file; they are not returned as I/O errors. Changing this error-reporting API does not change the wire encoding or format version.

An empty valid object without debug information occupies 96 bytes: 31 header bytes, 8 code-count bytes, 16 constant-section bytes, 24 global-storage bytes, 8 symbol-count bytes, 8 base-reference-count bytes, and 1 debug-presence byte. It contains no base-reference records.
