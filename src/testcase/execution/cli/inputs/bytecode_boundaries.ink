[abi("C")]
private func localC(): i32
{
  return 19;
}

private export "C" func inkPrivateEntryBoundary(): i32
{
  return 23;
}

// A native export remains a normal executable bytecode definition, including when selected as the CLI entry.
public export "C" func main(): i32
{
  return localC() + inkPrivateEntryBoundary();
}
