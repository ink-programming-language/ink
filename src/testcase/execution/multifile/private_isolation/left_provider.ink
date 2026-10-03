// This helper remains private even though another file defines the same name.
private func helper(): i32
{
  return 19;
}

// Only this wrapper is public; its direct call must retain this file's helper.
public func leftValue(): i32
{
  return helper();
}
