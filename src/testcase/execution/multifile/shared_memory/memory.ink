// A callee in another file writes into the caller-owned integer through its writable pointer.
func increase(Address: *i32, Amount: i32): void
{
  *Address = *Address + Amount;
}

// Reading through an imported function observes the latest write to the same storage.
func readValue(Address: *i32): i32
{
  return *Address;
}

// Returning an argument pointer preserves its provenance without taking ownership of its target.
func alias(Address: *i32): *i32
{
  return Address;
}
