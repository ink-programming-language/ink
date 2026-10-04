import "C" func inkReflectionNext(): i32;
import "C" func inkReflectionProbe(): i32;

class Unused
{
  field UnusedCode: i32;

  func __init__(InitialUnusedCode: i32): void
  {
    this.UnusedCode = InitialUnusedCode;
  }
};

class Counter
{
  field Value: i32 = inkReflectionNext();
  field Step: i32 = 2;
  field Flag: bool = true;
  field Flags: [bool; 2] = [true, false];
  private field Secret: i32 = 7;

  func advance(): i32
  {
    this.Value = this.Value + this.Step;
    return this.Value;
  }

  func copy(): Counter
  {
    return Counter(this.Value, this.Step);
  }

  func __init__(): void
  {
  }

  func __init__(InitialValue: i32): void
  {
    this.Value = InitialValue;
  }

  func __init__(InitialValue: i32, InitialStep: i32): void
  {
    this.Value = InitialValue;
    this.Step = InitialStep;
  }

  func __init__(InitialValue: i32, InitialStep: i32, InitialFlag: bool): void
  {
    this.Value = InitialValue;
    this.Step = InitialStep;
    this.Flag = InitialFlag;
  }

  func __init__(InitialValue: i32, InitialStep: i32, InitialFlag: bool, InitialFlags: [bool; 2]): void
  {
    this.Value = InitialValue;
    this.Step = InitialStep;
    this.Flag = InitialFlag;
    this.Flags = InitialFlags;
  }

  private func __init__(InitialValue: i32, InitialStep: i32, InitialFlag: bool, InitialFlags: [bool; 2], InitialSecret: i32): void
  {
    this.Value = InitialValue;
    this.Step = InitialStep;
    this.Flag = InitialFlag;
    this.Flags = InitialFlags;
    this.Secret = InitialSecret;
  }
};

func main(): i32
{
  return inkReflectionProbe();
}
