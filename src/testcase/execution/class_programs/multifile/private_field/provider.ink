// A private field can use its default during imported construction but cannot be read by the consumer.
class Point
{
  private field X: i32 = 42;

  func __init__(): void
  {
  }

  private func __init__(InitialX: i32): void
  {
    this.X = InitialX;
  }
};
