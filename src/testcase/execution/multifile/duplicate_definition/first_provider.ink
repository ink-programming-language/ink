// Two definitions of one signature in the same source module must be rejected.
func value(): i32
{
  return 19;
}

// A different body does not create a distinct overload when the signature is unchanged.
func value(): i32
{
  return 23;
}
