// Neither narrow unsigned overload is preferred for an unconstrained integer literal.
func choose(Value: u8): i32
{
  return 19;
}

// This distinct signature forms a legal overload set but ties with choose(u8) for literal 7.
func choose(Value: u16): i32
{
  return 23;
}
