// A module can publish a host import, a local C ABI function, and a native export independently.
public import "C" func abs(Value: i32): i32;

[abi("C")]
public func localValue(Value: i32): i32
{
  return Value + 1;
}

public export "C" func inkExportedValue(Value: i32): i32
{
  return Value + 2;
}

// Native export does not grant access through Ink source imports.
private export "C" func inkPrivateNativeValue(): i32
{
  return 3;
}

public func privateWrapped(): i32
{
  return inkPrivateNativeValue();
}
