// Independent compilation and dependency-graph analysis both resolve a later private helper.
public func answer(): i32
{
  return seed() + 1;
}

// The private helper remains callable by another definition in this object.
private func seed(): i32
{
  return 20;
}
