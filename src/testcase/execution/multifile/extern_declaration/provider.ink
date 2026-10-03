// External C declarations remain valid without an Ink body and resolve in the execution process.
import "C" func abs(Value: i32): i32;

// A public Ink wrapper can call the native declaration and be imported by another source file.
public func absolute(Value: i32): i32
{
  return abs(Value);
}
