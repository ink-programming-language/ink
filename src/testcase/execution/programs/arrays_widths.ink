// Preserve every integer element width while exercising the same width as a dynamic array index.
import "C" func puts(Text: *u8): i32;

// Read both i8 extrema and write the final element through an index and an element pointer.
func signedI8(): bool
{
  var Values: [i8; 3] = [-128, 0, 127];
  var First: i8 = 0;
  var Last: i8 = 2;
  var Maximum = Values[Last];
  var Address = &Values[Last];
  Values[First] = 127;
  *Address = -128;
  return Maximum == 127 && Values[First] == 127 && Values[1] == 0 && Values[Last] == -128;
}

// Read both u8 extrema and write the final element through an index and an element pointer.
func unsignedU8(): bool
{
  var Values: [u8; 3] = [0, 0, 255];
  var First: u8 = 0;
  var Last: u8 = 2;
  var Maximum = Values[Last];
  var Address = &Values[Last];
  Values[First] = 255;
  *Address = 0;
  return Maximum == 255 && Values[First] == 255 && Values[1] == 0 && Values[Last] == 0;
}

// Read both i16 extrema and write the final element through an index and an element pointer.
func signedI16(): bool
{
  var Values: [i16; 3] = [-32768, 0, 32767];
  var First: i16 = 0;
  var Last: i16 = 2;
  var Maximum = Values[Last];
  var Address = &Values[Last];
  Values[First] = 32767;
  *Address = -32768;
  return Maximum == 32767 && Values[First] == 32767 && Values[1] == 0 && Values[Last] == -32768;
}

// Read both u16 extrema and write the final element through an index and an element pointer.
func unsignedU16(): bool
{
  var Values: [u16; 3] = [0, 0, 65535];
  var First: u16 = 0;
  var Last: u16 = 2;
  var Maximum = Values[Last];
  var Address = &Values[Last];
  Values[First] = 65535;
  *Address = 0;
  return Maximum == 65535 && Values[First] == 65535 && Values[1] == 0 && Values[Last] == 0;
}

// Read both i32 extrema and write the final element through an index and an element pointer.
func signedI32(): bool
{
  var Values: [i32; 3] = [-2147483648, 0, 2147483647];
  var First: i32 = 0;
  var Last: i32 = 2;
  var Maximum = Values[Last];
  var Address = &Values[Last];
  Values[First] = 2147483647;
  *Address = -2147483648;
  return Maximum == 2147483647 && Values[First] == 2147483647 && Values[1] == 0 && Values[Last] == -2147483648;
}

// Read both u32 extrema and write the final element through an index and an element pointer.
func unsignedU32(): bool
{
  var Values: [u32; 3] = [0, 0, 4294967295];
  var First: u32 = 0;
  var Last: u32 = 2;
  var Maximum = Values[Last];
  var Address = &Values[Last];
  Values[First] = 4294967295;
  *Address = 0;
  return Maximum == 4294967295 && Values[First] == 4294967295 && Values[1] == 0 && Values[Last] == 0;
}

// Read both i64 extrema and write the final element through an index and an element pointer.
func signedI64(): bool
{
  var Values: [i64; 3] = [-9223372036854775808, 0, 9223372036854775807];
  var First: i64 = 0;
  var Last: i64 = 2;
  var Maximum = Values[Last];
  var Address = &Values[Last];
  Values[First] = 9223372036854775807;
  *Address = -9223372036854775808;
  return Maximum == 9223372036854775807 && Values[First] == 9223372036854775807 && Values[1] == 0 && Values[Last] == -9223372036854775808;
}

// Read both u64 extrema and write the final element through an index and an element pointer.
func unsignedU64(): bool
{
  var Values: [u64; 3] = [0, 0, 18446744073709551615];
  var First: u64 = 0;
  var Last: u64 = 2;
  var Maximum = Values[Last];
  var Address = &Values[Last];
  Values[First] = 18446744073709551615;
  *Address = 0;
  return Maximum == 18446744073709551615 && Values[First] == 18446744073709551615 && Values[1] == 0 && Values[Last] == 0;
}

// Read both i128 extrema and write the final element through an index and an element pointer.
func signedI128(): bool
{
  var Values: [i128; 3] = [-170141183460469231731687303715884105728, 0, 170141183460469231731687303715884105727];
  var First: i128 = 0;
  var Last: i128 = 2;
  var Maximum = Values[Last];
  var Address = &Values[Last];
  Values[First] = 170141183460469231731687303715884105727;
  *Address = -170141183460469231731687303715884105728;
  return Maximum == 170141183460469231731687303715884105727 && Values[First] == 170141183460469231731687303715884105727 && Values[1] == 0 && Values[Last] == -170141183460469231731687303715884105728;
}

// Read both u128 extrema and write the final element through an index and an element pointer.
func unsignedU128(): bool
{
  var Values: [u128; 3] = [0, 0, 340282366920938463463374607431768211455];
  var First: u128 = 0;
  var Last: u128 = 2;
  var Maximum = Values[Last];
  var Address = &Values[Last];
  Values[First] = 340282366920938463463374607431768211455;
  *Address = 0;
  return Maximum == 340282366920938463463374607431768211455 && Values[First] == 340282366920938463463374607431768211455 && Values[1] == 0 && Values[Last] == 0;
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
  puts("ArraysWidths.i8");
  if (!check(signedI8()))
  {
    return 1;
  }

  puts("ArraysWidths.u8");
  if (!check(unsignedU8()))
  {
    return 1;
  }

  puts("ArraysWidths.i16");
  if (!check(signedI16()))
  {
    return 1;
  }

  puts("ArraysWidths.u16");
  if (!check(unsignedU16()))
  {
    return 1;
  }

  puts("ArraysWidths.i32");
  if (!check(signedI32()))
  {
    return 1;
  }

  puts("ArraysWidths.u32");
  if (!check(unsignedU32()))
  {
    return 1;
  }

  puts("ArraysWidths.i64");
  if (!check(signedI64()))
  {
    return 1;
  }

  puts("ArraysWidths.u64");
  if (!check(unsignedU64()))
  {
    return 1;
  }

  puts("ArraysWidths.i128");
  if (!check(signedI128()))
  {
    return 1;
  }

  puts("ArraysWidths.u128");
  if (!check(unsignedU128()))
  {
    return 1;
  }

  return 0;
}
