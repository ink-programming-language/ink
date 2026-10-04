import "C" func inkReflectionNext(): i32;
import "C" func inkReflectionProbe(): i32;

class Unused
{
  field UnusedCode: i32;
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
};

func main(): i32
{
  return inkReflectionProbe();
}
