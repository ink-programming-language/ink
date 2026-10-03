// Each case records observable array evaluation order and verifies the resulting values.
import "C" func puts(Text: *u8): i32;

func next(Count: *i32): i32
{
  puts("next");
  *Count = *Count + 1;
  return *Count;
}

func pick(Count: *i32): i32
{
  puts("index");
  *Count = *Count + 1;
  return 1;
}

func right(Count: *i32): i32
{
  puts("rhs");
  var Before = *Count;
  *Count = *Count + 10;
  return Before;
}

func writeThenIndex(Element: *i32): i32
{
  puts("index_write");
  *Element = 42;
  return 0;
}

func mutate(Element: *i32): i32
{
  puts("mutate");
  *Element = 99;
  return 7;
}

func captured(Values: [i32; 2], Marker: i32): bool
{
  puts("capture");
  return Values[0] == 19 && Values[1] == 23 && Marker == 7;
}

func make(Value: i32): [i32; 2]
{
  puts("make");
  return [Value, Value + 1];
}

// Every literal element is captured before evaluation advances to the next element.
func literalOrder(): bool
{
  var Count = 0;
  var Values = [next(&Count), next(&Count), next(&Count)];
  return Values[0] == 1 && Values[1] == 2 && Values[2] == 3 && Count == 3;
}

// Nested literal evaluation completes each row in source order.
func nestedLiteralOrder(): bool
{
  var Count = 0;
  var Values = [[next(&Count), next(&Count)], [next(&Count), next(&Count)]];
  return Values[0][0] == 1 && Values[0][1] == 2 && Values[1][0] == 3 && Values[1][1] == 4 && Count == 4;
}

// One evaluated initializer supplies all copies in a repeated array.
func repeatOnce(): bool
{
  var Count = 0;
  var Values = [next(&Count); 4];
  return Values[0] == 1 && Values[1] == 1 && Values[2] == 1 && Values[3] == 1 && Count == 1;
}

// An empty repeated array still evaluates its initializer exactly once.
func zeroRepeatOnce(): bool
{
  var Count = 0;
  var Empty = [next(&Count); 0];
  return Count == 1;
}

// The RHS observes the one index evaluation and the final store reuses its result.
func assignmentOrder(): bool
{
  var Count = 0;
  var Values = [0, 0];
  Values[pick(&Count)] = right(&Count);
  return Values[0] == 0 && Values[1] == 1 && Count == 11;
}

// A read takes place after an index expression changes the addressed element.
func loadAfterIndex(): bool
{
  var Values = [0];
  var Result = Values[writeThenIndex(&Values[0])];
  return Result == 42 && Values[0] == 42;
}

// An array argument is a snapshot taken before the following argument mutates its source.
func argumentCapture(): bool
{
  var Values = [19, 23];
  var Result = captured(Values, mutate(&Values[0]));
  return Result && Values[0] == 99 && Values[1] == 23;
}

// A returned array retains both runtime-produced elements after its callee returns.
func runtimeReturn(): bool
{
  var Values = make(41);
  return Values[0] == 41 && Values[1] == 42;
}

// Indexing an unnamed array return does not replay its producing function.
func temporaryReturn(): bool
{
  return make(41)[1] == 42;
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
  puts("ArraysEffects.literal_order");
  if (!check(literalOrder())) return 1;

  puts("ArraysEffects.nested_literal_order");
  if (!check(nestedLiteralOrder())) return 1;

  puts("ArraysEffects.repeat_once");
  if (!check(repeatOnce())) return 1;

  puts("ArraysEffects.zero_repeat_once");
  if (!check(zeroRepeatOnce())) return 1;

  puts("ArraysEffects.assignment_index_before_rhs");
  if (!check(assignmentOrder())) return 1;

  puts("ArraysEffects.load_after_index");
  if (!check(loadAfterIndex())) return 1;

  puts("ArraysEffects.argument_capture");
  if (!check(argumentCapture())) return 1;

  puts("ArraysEffects.runtime_array_return");
  if (!check(runtimeReturn())) return 1;

  puts("ArraysEffects.temporary_array_return");
  if (!check(temporaryReturn())) return 1;

  return 0;
}
