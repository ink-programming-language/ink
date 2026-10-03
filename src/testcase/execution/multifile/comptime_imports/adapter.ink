from provider import seed;

// Calling this ordinary imported function at comptime also needs its transitive dependency body.
func answer(): i32
{
  return seed() + 2;
}
