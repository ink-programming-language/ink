// Translated from solution.cpp.

func main() -> dynamic
{
  var S: dynamic = cpp_uninitialized();
  read(S);
  var c0: dynamic = cpp_uninitialized();
  var c1: dynamic = 0;
  for (var i: dynamic in S)
  {
    if ((i == cpp_char("0")))
    {
      c0 += 1;
    } else
    {
      c1 += 1;
    }
  }
  write((2 * min(c0, c1)), "\n");
}
