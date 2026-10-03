// This helper has the same spelling and signature as the other provider's private helper.
private func helper(): i32
{
  return 23;
}

// This wrapper must call its own helper after final function IDs are assigned.
func rightValue(): i32
{
  return helper();
}
