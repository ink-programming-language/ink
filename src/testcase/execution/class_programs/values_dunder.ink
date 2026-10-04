// Operator receivers are value snapshots; ordinary object and nested-field copies remain independent.
import "C" func puts(Text: *u8): i32;

class Number
{
  field Value: i32;

  func __add__(Other: Number): Number
  {
    this.Value = this.Value + Other.Value;
    return Number(this.Value);
  }

  func __eq__(Other: Number): bool
  {
    return this.Value == Other.Value;
  }

  func __neg__(): i32
  {
    return this.Value;
  }

  func __init__(InitialValue: i32): void
  {
    this.Value = InitialValue;
  }
};

class Box
{
  field Current: Number;
  field Samples: [Number; 2];

  func __init__(InitialCurrent: Number, InitialSamples: [Number; 2]): void
  {
    this.Current = InitialCurrent;
    this.Samples = InitialSamples;
  }
};

class Borrow
{
  field Address: *i32;

  func __init__(InitialAddress: *i32): void
  {
    this.Address = InitialAddress;
  }
};

func copy(Value: Number): Number
{
  var Result = Value;
  Result.Value = 7;
  return Result;
}

func main(): i32
{
  puts("Class.dunder_snapshot");
  const Original = Number(20);
  const Sum = Original + Number(22);
  const Changed = copy(Original);
  if (!(Sum == Number(42)) || -Sum != 42 || Original.Value != 20 || Changed.Value != 7)
  {
    return 1;
  }
  puts("PASS");
  puts("Class.nested_alias");
  var B = Box(Number(1), [Number(2), Number(3)]);
  var Saved = &B.Current.Value;
  const Before = B;
  B = Box(Number(40), [Number(1), Number(2)]);
  B.Samples[1].Value = 2;
  if (*Saved + B.Samples[1].Value != 42 || Before.Current.Value != 1)
  {
    return 2;
  }
  puts("PASS");
  puts("Class.pointer_field_copy");
  var Target = 0;
  var First = Borrow(&Target);
  const Second = First;
  var Alias = Second.Address;
  *Alias = 42;
  if (Target != 42)
  {
    return 3;
  }
  puts("PASS");
  return 0;
}
