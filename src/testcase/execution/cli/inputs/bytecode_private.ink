// Source visibility prevents CLI metadata options from promoting this internal helper.
private func hidden(): i32
{
  return 42;
}

// An unmodified top-level function is public and may call the private function in its own file.
func main(): i32
{
  return hidden();
}
