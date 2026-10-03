// An ordinary integer literal prefers this i32 overload.
func pick(Value: i32): i32
{
  return Value + 10;
}

// A typed u8 argument selects this definition without widening to i32.
func pick(Value: u8): i32
{
  if (Value == 7)
  {
    return 20;
  }
  return 0;
}

// A bool parameter remains distinct from both integer signatures.
func pick(Value: bool): i32
{
  if (Value)
  {
    return 30;
  }
  return 31;
}

// Arity also participates in the exported signature identity.
func pick(): i32
{
  return 4;
}
