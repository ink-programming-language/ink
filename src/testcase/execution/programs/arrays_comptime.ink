// Compile-time array operations are frozen and checked through a normal runtime main.
import "C" func puts(Text: *u8): i32;

// Nested writes update one leaf while preserving all siblings.
func nestedWrites(): bool
{
  comptime var Values = [[1, 2], [3, 4]];
  comptime
  {
    Values[1][0] = 40;
    Values[0][1] = 2;
  }
  return comptime (Values[0][0] == 1 && Values[0][1] == 2 && Values[1][0] == 40 && Values[1][1] == 4);
}

// Copies and frozen constants retain their own nested values after either mutable array changes.
func independentCopies(): bool
{
  comptime var Original = [[10, 20], [30, 40]];
  comptime var Copy = Original;
  comptime const Frozen = Original;
  comptime
  {
    Original[0][0] = 100;
    Copy[1][1] = 42;
  }
  return comptime (Original[0][0] == 100 && Original[1][1] == 40 && Copy[0][0] == 10 && Copy[1][1] == 42 && Frozen[0][0] == 10 && Frozen[1][1] == 40);
}

// Arithmetic compound assignments update the selected element without touching the other element.
func compoundArithmetic(): bool
{
  comptime var Values = [10, 99];
  comptime
  {
    Values[0] += 7;
    Values[0] -= 2;
    Values[0] *= 4;
    Values[0] /= 3;
    Values[0] %= 6;
  }
  return comptime (Values[0] == 2 && Values[1] == 99);
}

// Bitwise compound assignments and shifts share the same indexed element.
func compoundBitwise(): bool
{
  comptime var Values = [[5, 99]];
  comptime
  {
    Values[0][0] <<= 3;
    Values[0][0] |= 3;
    Values[0][0] ^= 9;
    Values[0][0] &= 62;
    Values[0][0] >>= 1;
  }
  return comptime (Values[0][0] == 17 && Values[0][1] == 99);
}

// A local compile-time length controls both the declared type and repeated initializer.
func localLength(): bool
{
  comptime const Count: u8 = 3;
  var Values: [u8; Count + 1] = [42; Count + 1];
  return Values[0] == 42 && Values[1] == 42 && Values[2] == 42 && Values[3] == 42;
}

// A compile-time function mutates its local array before returning a frozen aggregate.
comptime func make(): [i32; 2]
{
  var Values = [1, 0];
  Values[1] = 42;
  return Values;
}

func frozenReturn(): bool
{
  const Frozen = comptime make();
  var Copy = Frozen;
  Copy[1] = 99;
  return Frozen[0] == 1 && Frozen[1] == 42 && Copy[1] == 99;
}

// Repetition evaluates a compile-time update once before copying the returned value.
func repeatedSideEffect(): bool
{
  comptime var Count: i32 = 0;
  comptime var Values = [++Count; 3];
  return comptime (Count == 1 && Values[0] == 1 && Values[1] == 1 && Values[2] == 1);
}

// Zero-length repetition keeps its initializer's side effect even though no elements are retained.
func zeroRepeatedSideEffect(): bool
{
  comptime var Count: i32 = 0;
  comptime var Empty = [++Count; 0];
  return comptime (Count == 1);
}

// An indexed RHS assignment must survive when the outer assignment stores into its earlier-selected element.
func siblingAssignment(): bool
{
  comptime var Index: i32 = 0;
  comptime var Values = [0, 0];
  comptime
  {
    Values[Index++] = Values[Index] = 21;
  }
  return comptime (Index == 1 && Values[0] == 21 && Values[1] == 21);
}

func check(Condition: bool): bool
{
  if (Condition)
  {
    puts("PASS");
    return true;
  }
  puts("FAIL");
  return false;
}

func main(): i32
{
  puts("ArraysComptime.nested_writes");
  if (!check(nestedWrites())) return 1;

  puts("ArraysComptime.independent_copies");
  if (!check(independentCopies())) return 1;

  puts("ArraysComptime.compound_arithmetic");
  if (!check(compoundArithmetic())) return 1;

  puts("ArraysComptime.compound_bitwise");
  if (!check(compoundBitwise())) return 1;

  puts("ArraysComptime.local_const_length");
  if (!check(localLength())) return 1;

  puts("ArraysComptime.frozen_function_return");
  if (!check(frozenReturn())) return 1;

  puts("ArraysComptime.repeated_side_effect");
  if (!check(repeatedSideEffect())) return 1;

  puts("ArraysComptime.zero_repeat_side_effect");
  if (!check(zeroRepeatedSideEffect())) return 1;

  puts("ArraysComptime.sibling_assignment");
  if (!check(siblingAssignment())) return 1;

  return 0;
}
