// Compile-time C results flow into runtime checks without replaying their effects.
import "C" func puts(Text: *u8): i32;
import "C" func abs(Value: i32): i32;
import "C" func atoi(Text: *u8): i32;
import "C" func strcmp(Left: *u8, Right: *u8): i32;
import "C" func InkMissingExternalProgramSymbola57e3f8d(): bool;

comptime const Magnitude: i32 = abs(-37);
comptime const Emitted: i32 = puts("External.comptime_initializer");

comptime func compute(Value: i32): i32
{
  return abs(Value) + abs(-5);
}

func ordinary(Value: i32): i32
{
  return abs(Value);
}

func readFirst(): i32
{
  return comptime Magnitude;
}

func readSecond(): i32
{
  return comptime Magnitude;
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
  // Multiple runtime readers reuse the same module compile-time value.
  puts("External.comptime_stored");
  if (!check(readFirst() == 37 && readSecond() == 37)) return 1;

  // Reusing the native initializer's result must not emit its message again.
  puts("External.comptime_effect_once");
  if (!check((comptime Emitted) >= 0 && (comptime Emitted) >= 0)) return 1;

  // A compile-time function runs native instructions with its actual parameter.
  puts("External.comptime_function");
  if (!check((comptime compute(-7)) == 12)) return 1;

  // Ordinary Ink functions can also execute native calls when invoked at compile time.
  puts("External.comptime_ordinary");
  if (!check((comptime ordinary(-19)) == 19)) return 1;

  // A direct nested native expression uses the first call's signed return value.
  puts("External.comptime_nested");
  if (!check((comptime abs(atoi("-81"))) == 81)) return 1;

  // Multiple C string arguments are independently marshalled at compile time.
  puts("External.comptime_strings");
  if (!check((comptime strcmp("same", "same")) == 0)) return 1;

  // A skipped external call does not attempt to resolve its missing host symbol.
  puts("External.comptime_skipped_symbol");
  if (!check(comptime (true || InkMissingExternalProgramSymbola57e3f8d()))) return 1;

  return 0;
}
