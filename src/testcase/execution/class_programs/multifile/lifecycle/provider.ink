class Counter
{
  field Target: *i32;

  func __init__(Target: *i32): void
  {
    this.Target = Target;
    *this.Target = *this.Target + 2;
  }

  func __del__(): void
  {
    *this.Target = *this.Target + 40;
  }
};
