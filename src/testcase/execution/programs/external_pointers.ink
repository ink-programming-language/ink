// Keep all CString buffers in the live main activation while reusing C pointers.
extern "C" func puts(Text: *u8): i32;
extern "C" func strlen(Text: *u8): u64;
extern "C" func strcmp(Left: *u8, Right: *u8): i32;
extern "C" func strchr(Text: *u8, Needle: i32): *u8;
extern "C" func strrchr(Text: *u8, Needle: i32): *u8;
extern "C" func strstr(Text: *u8, Needle: *u8): *u8;
extern "C" func strcpy(Destination: *u8, Source: *u8): *u8;
extern "C" func memchr(Buffer: *u8, Needle: i32, Count: u64): *u8;

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
  // An interior pointer survives local storage and can be passed to two C calls.
  puts("External.pointer_local");
  var Tail = strchr("prefix:tail", 58);
  if (!check(strlen(Tail) == 5 && strcmp(Tail, ":tail") == 0)) return 1;

  // A returned pointer also works directly as the next native call's argument.
  puts("External.pointer_nested");
  if (!check(strlen(strchr("alpha:beta", 58)) == 5)) return 1;

  // Searching for zero returns the terminator, which is a valid empty C string.
  puts("External.pointer_terminator");
  if (!check(strlen(strchr("tail", 0)) == 0)) return 1;

  // A second search function preserves the offset of the last occurrence.
  puts("External.pointer_last_match");
  if (!check(strcmp(strrchr("a:b:c", 58), ":c") == 0)) return 1;

  // A two-pointer call returns a pointer into the correct source buffer.
  puts("External.pointer_substring");
  if (!check(strcmp(strstr("prefix:needle:end", "needle"), "needle:end") == 0)) return 1;

  // Native writes affect the returned writable copy and leave a fresh literal intact.
  puts("External.pointer_mutation");
  var Mutated = strcpy("123456789", "ink");
  if (!check(strcmp(Mutated, "ink") == 0 && strlen("123456789") == 9)) return 1;

  // Identical interned strings produce distinct writable native buffers per conversion.
  puts("External.pointer_independent_copies");
  var First = strchr("ABC", 65);
  var Second = strchr("ABC", 65);
  strcpy(First, "Z");
  if (!check(strcmp(First, "Z") == 0 && strcmp(Second, "ABC") == 0)) return 1;

  // Copying an existing pointer preserves aliasing instead of copying its bytes.
  puts("External.pointer_alias");
  var Alias = First;
  strcpy(Alias, "Q");
  if (!check(strcmp(First, "Q") == 0 && strcmp(Second, "ABC") == 0)) return 1;

  // A three-argument native search preserves both the count and result offset.
  puts("External.pointer_counted_search");
  if (!check(strlen(memchr("ABC", 66, 3)) == 2)) return 1;

  return 0;
}
