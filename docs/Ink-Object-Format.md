# Ink object binary format

This format serializes the logical contents of `ink::vm::ObjectFile`. Its default format and instruction-set versions are both `1.0.0`; the supported versions are read through `ConfigManager`.

Fields appear in the order below. There is no implicit padding, native pointer, C++ object layout, or terminating byte after a string. All sizes and offsets are byte counts. Section alignment describes loaded storage and does not insert padding into the file.

`u8`, `u16`, `u32`, and `u64` denote unsigned integers of exactly that many bits. The byte-order field controls every multibyte integer in the container. Code, constants, global initializers, and strings are raw byte sequences and are copied verbatim. Code must already contain instructions encoded in the declared byte order; these APIs do not encode individual instructions.

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

Local and exported symbols require definitions. Imported symbols have none. Import and export names share the object's external namespace.

## Base references

A `u64` entry count is followed by that many records:

| Field | Encoding |
| --- | --- |
| Symbol index present | `u8`: absent `0`, present `1` |
| Symbol index, if present | `u32` |

Entries `0`, `1`, and `2` are required and have no symbol index: they represent BP, SP, and PC. Subsequent entries require valid object-local symbol indices. Counts of index-addressable tables must fit the model's 32-bit indices.

## Debug information

A `u8` presence flag (`0` or `1`) follows the base references. An absent debug record ends the object. A present record contains:

1. A `u64` source-file count, followed by each source name as a `u64` byte count and that many raw string bytes.
2. A `u64` line-record count, followed by records with `u64` code offset, `u32` file index, `u32` line, and `u32` column, in that order.

Code offsets address the whole code buffer. Source-file indices are object local; line and column numbers start at one. A present but empty debug record is distinct from an absent record.

## Reading and writing

`ObjectFileWriter::write()` returns the encoded bytes. `writeFile()` validates and encodes the object before opening the destination in binary truncation mode. It reports open, write, and close failures explicitly.

`ObjectFileReader::read()` returns an owned, validated object from a byte span. `readFile()` reads a binary file and applies the same decoder. A buffer or file contains exactly one object; trailing bytes are rejected.

Both classes report failures through `std::expected` and `ObjectFileIOError`. Structural validation errors retain the `ObjectFileValidator` error kind and record index. Decoder errors carry a byte offset. Lengths and counts are checked against the remaining input before allocating their contents.

An empty valid object without debug information occupies 99 bytes, including the three empty base-reference entries.
