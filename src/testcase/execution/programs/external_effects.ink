// Trace native side effects and argument order, and observe void/no-argument CRT calls.
extern "C" func puts(Text: *u8): i32;
extern "C" func abs(Value: i32): i32;
extern "C" func srand(Seed: u32): void;
extern "C" func rand(): i32;
extern "C" func strchr(Text: *u8, Needle: i32): *u8;
extern "C" func strcmp(Left: *u8, Right: *u8): i32;
extern "C" func memset(Buffer: *u8, Byte: i32, Count: u64): *u8;

func emit(): i32
{
  puts("native");
  return 7;
}

func markBuffer(Buffer: *u8): *u8
{
  puts("buffer");
  return Buffer;
}

func markByte(): i32
{
  puts("byte");
  return 88;
}

func markCount(): u64
{
  puts("count");
  return 2;
}

func nested(): i32
{
  puts("outer");
  return abs(emit());
}

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
  // The same source function executes its native output once per invocation.
  puts("External.effects_repeated");
  if (!check(abs(emit()) + abs(emit()) == 14)) return 1;

  // Reusing an already computed value does not replay its external call.
  puts("External.effects_reuse");
  var Saved = emit();
  if (!check(Saved + Saved == 14)) return 1;

  // Three typed native arguments are evaluated from left to right, exactly once.
  puts("External.effects_arguments");
  var Buffer = strchr("ABC", 65);
  var Changed = memset(markBuffer(Buffer), markByte(), markCount());
  if (!check(strcmp(Changed, "XXC") == 0)) return 1;

  // A void native call changes state subsequently read by zero-argument calls.
  // Compare reseeded sequences instead of depending on a CRT-specific random value.
  puts("External.effects_void_and_zero_args");
  srand(1234);
  var First = rand();
  var Second = rand();
  srand(1234);
  if (!check(rand() == First && rand() == Second)) return 1;

  // Native effects in nested Ink calls occur before the enclosing result check.
  puts("External.effects_nested_calls");
  if (!check(nested() == 7)) return 1;

  return 0;
}
