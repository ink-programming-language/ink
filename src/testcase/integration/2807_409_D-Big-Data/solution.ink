// Translated from solution.cpp.

var s: dynamic = "1001010111001010";

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  write(s[(n - 1)], "\n");
  return 0;
}
