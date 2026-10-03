// Verify value copies, stable element aliases, function boundaries and pointer-valued arrays.
import "C" func puts(Text: *u8): i32;

func makeValues(Value: i32): [i32; 2]
{
  return [10, Value];
}

func readSecond(Values: [i32; 2]): i32
{
  return Values[1];
}

func copyAndChange(Values: [i32; 2]): [i32; 2]
{
  var Copy = Values;
  Copy[0] = 32;
  return Copy;
}

func makeBytes(): [u8; 2]
{
  return [0, 255];
}

func readByte(Values: [u8; 2]): u8
{
  return Values[1];
}

func setArray(Values: *[i32; 2]): void
{
  (*Values)[0] = 10;
  (*Values)[1] = 32;
}

func copyPointers(Values: [*i32; 2]): [*i32; 2]
{
  return Values;
}

func mutateCallerReturnSnapshot(Values: [i32; 2], Original: *[i32; 2]): [i32; 2]
{
  (*Original)[0] = 99;
  (*Original)[1] = 100;
  return Values;
}

func copyAndChangeMatrix(Values: [[i32; 2]; 2]): [[i32; 2]; 2]
{
  var Copy = Values;
  Copy[1] = [40, 42];
  return Copy;
}

func selectRow(Values: [[i32; 2]; 2], Index: u128): [i32; 2]
{
  return Values[Index];
}

// Verify independent copies without relying on pointer equality.
func independentCopies(): bool
{
  var Original = [10, 20];
  var Copy = Original;
  Original[0] = 99;
  Copy[1] = 32;
  return Original[0] == 99 && Original[1] == 20 && Copy[0] == 10 && Copy[1] == 32;
}

// Verify nested copies without relying on pointer equality.
func nestedCopies(): bool
{
  var Original = [[1, 2], [3, 4]];
  var Copy = Original;
  Original[0][0] = 99;
  Copy[1][1] = 42;
  return Original[0][0] == 99 && Original[1][1] == 4 && Copy[0][0] == 1 && Copy[1][1] == 42;
}

// Verify whole array assignment without relying on pointer equality.
func wholeArrayAssignment(): bool
{
  var Original = [10, 20];
  var Copy: [i32; 2];
  Copy = Original;
  Copy[0] = 42;
  Original = [7, 8];
  return Original[0] == 7 && Original[1] == 8 && Copy[0] == 42 && Copy[1] == 20;
}

// Verify stable element alias without relying on pointer equality.
func stableElementAlias(): bool
{
  var Values = [0, 0];
  var First = &Values[0];
  var Second = &Values[1];
  Values = [10, 32];
  *First = *First + 1;
  return Values[0] == 11 && *First == 11 && Values[1] == 32 && *Second == 32;
}

// Verify stable nested alias without relying on pointer equality.
func stableNestedAlias(): bool
{
  var Values = [[0, 0], [0, 0]];
  var Element = &Values[1][0];
  Values = [[1, 2], [40, 4]];
  *Element = *Element + 2;
  return Values[0][0] == 1 && Values[1][0] == 42 && *Element == 42;
}

// Verify parameters and returns without relying on pointer equality.
func parametersAndReturns(): bool
{
  var Original = makeValues(42);
  var Changed = copyAndChange(Original);
  return readSecond(Original) == 42 && Original[0] == 10 && Changed[0] == 32 && Changed[1] == 42 && readByte([1, 255]) == 255;
}

// Verify temporary array reads without relying on pointer equality.
func temporaryArrayReads(): bool
{
  var Index: u128 = 1;
  return makeValues(42)[Index] == 42 && copyAndChange([10, 20])[0] == 32 && makeBytes()[1] == 255;
}

// Verify constant array reads without relying on pointer equality.
func constantArrayReads(): bool
{
  const Values: [u8; 3] = [0, 128, 255];
  const Matrix = [[10, 20], [30, 42]];
  var Index = 2;
  return Values[Index] == 255 && Values[0] == 0 && Matrix[1][1] == 42;
}

// Verify pointer element copies without relying on pointer equality.
func pointerElementCopies(): bool
{
  var First = 10;
  var Second = 20;
  var Pointers = [&First, &Second];
  var Copy = copyPointers(Pointers);
  *Copy[0] = 42;
  Pointers[0] = &Second;
  *Pointers[0] = 32;
  return First == 42 && Second == 32 && *Copy[0] == 42 && *Copy[1] == 32 && *Pointers[0] == 32;
}

// Verify pointer to array without relying on pointer equality.
func pointerToArray(): bool
{
  var Values = [0, 0];
  var Pointer = &Values;
  setArray(Pointer);
  var Element = &(*Pointer)[1];
  *Element = *Element + 10;
  return (*Pointer)[0] == 10 && (*Pointer)[1] == 42 && Values[1] == 42;
}

// A by-value parameter retains its snapshot when the callee also mutates the caller through an alias.
func parameterSnapshotDuringMutation(): bool
{
  var Original = [10, 42];
  var Returned = mutateCallerReturnSnapshot(Original, &Original);
  Returned[0] = 32;
  return Original[0] == 99 && Original[1] == 100 && Returned[0] == 32 && Returned[1] == 42;
}

// Nested parameters, returned matrices and extracted rows each preserve independent array values.
func nestedParametersAndReturns(): bool
{
  var Original = [[1, 2], [3, 4]];
  var Returned = copyAndChangeMatrix(Original);
  var Index: u128 = 1;
  var Row = selectRow(Returned, Index);
  var Alias = &Returned[1];
  *Alias = [50, 60];
  Original[1][1] = 99;
  return Original[0][0] == 1 && Original[1][0] == 3 && Original[1][1] == 99 && Returned[0][1] == 2 && Returned[1][0] == 50 && Returned[1][1] == 60 && Row[0] == 40 && Row[1] == 42;
}

// Repeated calls create independent array results even when the callee reuses the same local layout.
func repeatedCallsFreshArrays(): bool
{
  var First = makeValues(41);
  var Second = makeValues(42);
  var Third = copyAndChange(First);
  var Alias = &First[1];
  Second[1] = 99;
  Third[1] = 100;
  return First[0] == 10 && First[1] == 41 && *Alias == 41 && Second[0] == 10 && Second[1] == 99 && Third[0] == 32 && Third[1] == 100;
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
  puts("ArraysValues.independent_copies");
  if (!check(independentCopies()))
  {
    return 1;
  }

  puts("ArraysValues.nested_copies");
  if (!check(nestedCopies()))
  {
    return 1;
  }

  puts("ArraysValues.whole_array_assignment");
  if (!check(wholeArrayAssignment()))
  {
    return 1;
  }

  puts("ArraysValues.stable_element_alias");
  if (!check(stableElementAlias()))
  {
    return 1;
  }

  puts("ArraysValues.stable_nested_alias");
  if (!check(stableNestedAlias()))
  {
    return 1;
  }

  puts("ArraysValues.parameters_and_returns");
  if (!check(parametersAndReturns()))
  {
    return 1;
  }

  puts("ArraysValues.temporary_array_reads");
  if (!check(temporaryArrayReads()))
  {
    return 1;
  }

  puts("ArraysValues.constant_array_reads");
  if (!check(constantArrayReads()))
  {
    return 1;
  }

  puts("ArraysValues.pointer_element_copies");
  if (!check(pointerElementCopies()))
  {
    return 1;
  }

  puts("ArraysValues.pointer_to_array");
  if (!check(pointerToArray()))
  {
    return 1;
  }

  puts("ArraysValues.parameter_snapshot_during_mutation");
  if (!check(parameterSnapshotDuringMutation()))
  {
    return 1;
  }

  puts("ArraysValues.nested_parameters_and_returns");
  if (!check(nestedParametersAndReturns()))
  {
    return 1;
  }

  puts("ArraysValues.repeated_calls_fresh_arrays");
  if (!check(repeatedCallsFreshArrays()))
  {
    return 1;
  }

  return 0;
}
