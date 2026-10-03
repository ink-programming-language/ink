// These size_t signatures target the supported 64-bit Windows and Linux hosts.
import "C" func puts(Text: *u8): i32;
import "C" func strlen(Text: *u8): u64;
import "C" func strcmp(Left: *u8, Right: *u8): i32;
import "C" func memcmp(Left: *void, Right: *void, Count: u64): i32;

func check(Result: bool): bool
{
  if (Result)
  {
    puts("PASS");
    return true;
  }
  puts("FAIL");
  return false;
}

func main(): i32
{
  // Both empty and nonempty literals acquire a native trailing NUL.
  puts("External.string_empty");
  if (!check(strlen("") == 0)) return 1;

  puts("External.string_ascii");
  if (!check(strlen("Ink test") == 8)) return 1;

  // Native length counts UTF-8 bytes, not Unicode scalar values.
  puts("External.string_utf8");
  if (!check(strlen("中文") == 6)) return 1;

  // Direct compile-time calls preserve embedded NULs for C string consumers.
  puts("External.string_embedded_nul");
  if (!check((comptime strlen("ab\0cd")) == 2)) return 1;

  // Check strcmp's sign, whose exact nonzero magnitude differs between CRTs.
  puts("External.string_equal");
  if (!check(strcmp("same", "same") == 0)) return 1;

  puts("External.string_less");
  if (!check(strcmp("alpha", "beta") < 0)) return 1;

  puts("External.string_greater");
  if (!check(strcmp("beta", "alpha") > 0)) return 1;

  // Direct compile-time void-pointer arguments retain bytes after embedded NULs.
  puts("External.bytes_after_nul");
  if (!check((comptime memcmp("\0A", "\0B", 2)) < 0)) return 1;

  // Counted reads respect the explicit bound rather than the full literal length.
  puts("External.bytes_prefix");
  if (!check((comptime memcmp("\0A", "\0B", 1)) == 0)) return 1;

  // The implicit terminator matches an explicit NUL at the same byte position.
  puts("External.bytes_terminator");
  if (!check((comptime memcmp("ABC", "ABC\0", 4)) == 0)) return 1;

  return 0;
}
