// Calling a void function cannot produce the i32 value required by main's return.
func empty(): void
{
}

func main(): i32
{
  return empty();
}
