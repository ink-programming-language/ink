// Independently execute array construction, inference, repetition and nested/empty layouts.
import "C" func puts(Text: *u8): i32;

func emptyBytes(): [u8; 0]
{
  return [];
}

func acceptEmptyBytes(Values: [u8; 0]): bool
{
  return true;
}

func acceptEmptyIntegers(Values: [i32; 0]): bool
{
  return true;
}

func identityEmptyIntegers(Values: [i32; 0]): [i32; 0]
{
  return Values;
}

func selectEmptyRow(Values: [[i32; 0]; 3], Index: u128): [i32; 0]
{
  return Values[Index];
}

// Verify inferred literals through observable element values.
func inferredLiterals(): bool
{
  var Values = [10, 20, 30];
  return Values[0] == 10 && Values[1] == 20 && Values[2] == 30 && [7, 42][1] == 42;
}

// Verify contextual elements through observable element values.
func contextualElements(): bool
{
  var Bytes: [u8; 3] = [0, 128, 255];
  var Signed: [i8; 2] = [-128, 127];
  Bytes[0] = 42;
  return Bytes[0] == 42 && Bytes[1] == 128 && Bytes[2] == 255 && Signed[0] == -128 && Signed[1] == 127;
}

// Verify typed value inference through observable element values.
func typedValueInference(): bool
{
  var Byte: u8 = 255;
  var Values = [0, Byte];
  var Minimum: i16 = -32768;
  var Signed = [Minimum, 32767];
  return Values[0] == 0 && Values[1] == 255 && Signed[0] == -32768 && Signed[1] == 32767;
}

// Verify boolean elements through observable element values.
func booleanElements(): bool
{
  var Values = [false, true, false];
  var Index = 2;
  Values[Index] = true;
  return !Values[0] && Values[1] && Values[2];
}

// Verify repeated values through observable element values.
func repeatedValues(): bool
{
  var Values = [42; 4];
  var Byte: u8 = 255;
  var Bytes = [Byte; 3];
  Values[1] = 7;
  return Values[0] == 42 && Values[1] == 7 && Values[3] == 42 && Bytes[0] == 255 && Bytes[2] == 255;
}

// Verify compiletime length through observable element values.
func compiletimeLength(): bool
{
  comptime const Count = 2;
  var Values: [u8; Count + 1] = [42; Count + 1];
  var Index: u64 = 2;
  return Values[0] == 42 && Values[Index] == 42;
}

// Verify nested elements through observable element values.
func nestedElements(): bool
{
  var Matrix: [[u8; 2]; 3] = [[1, 2], [3, 4], [5, 6]];
  var Row: u8 = 2;
  var Column: i64 = 1;
  Matrix[Row][Column] = 42;
  return Matrix[0][0] == 1 && Matrix[1][1] == 4 && Matrix[2][0] == 5 && Matrix[Row][Column] == 42;
}

// Verify nested repetition through observable element values.
func nestedRepetition(): bool
{
  var Matrix = [[0, 1]; 3];
  Matrix[1][0] = 42;
  return Matrix[0][0] == 0 && Matrix[1][0] == 42 && Matrix[2][0] == 0 && Matrix[2][1] == 1;
}

// Verify empty arrays through observable element values.
func emptyArrays(): bool
{
  var First: [u8; 0] = [];
  var Second = [42; 0];
  var Third = First;
  First = emptyBytes();
  return acceptEmptyBytes(Third) && acceptEmptyBytes(First) && acceptEmptyIntegers(Second);
}

// Verify zero length inner arrays through observable element values.
func zeroLengthInnerArrays(): bool
{
  var Matrix = [[0; 0]; 3];
  var Empty = Matrix[2];
  var Row = &Matrix[1];
  *Row = [0; 0];
  var Index: u128 = 2;
  var Returned = identityEmptyIntegers(selectEmptyRow(Matrix, Index));
  return acceptEmptyIntegers(Empty) && acceptEmptyIntegers(Matrix[0]) && acceptEmptyIntegers(*Row) && acceptEmptyIntegers(Returned);
}

func check(Actual: bool): bool
{
  if (Actual)
  {
    puts("PASS");
    return true;
  }
  puts("FAIL");
  return false;
}

func main(): i32
{
  puts("ArraysBasics.inferred_literals");
  if (!check(inferredLiterals()))
  {
    return 1;
  }

  puts("ArraysBasics.contextual_elements");
  if (!check(contextualElements()))
  {
    return 1;
  }

  puts("ArraysBasics.typed_value_inference");
  if (!check(typedValueInference()))
  {
    return 1;
  }

  puts("ArraysBasics.boolean_elements");
  if (!check(booleanElements()))
  {
    return 1;
  }

  puts("ArraysBasics.repeated_values");
  if (!check(repeatedValues()))
  {
    return 1;
  }

  puts("ArraysBasics.compiletime_length");
  if (!check(compiletimeLength()))
  {
    return 1;
  }

  puts("ArraysBasics.nested_elements");
  if (!check(nestedElements()))
  {
    return 1;
  }

  puts("ArraysBasics.nested_repetition");
  if (!check(nestedRepetition()))
  {
    return 1;
  }

  puts("ArraysBasics.empty_arrays");
  if (!check(emptyArrays()))
  {
    return 1;
  }

  puts("ArraysBasics.zero_length_inner_arrays");
  if (!check(zeroLengthInnerArrays()))
  {
    return 1;
  }

  return 0;
}
