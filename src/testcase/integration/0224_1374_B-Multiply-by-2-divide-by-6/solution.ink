// Translated from solution.cpp.

var TestCases: dynamic = cpp_uninitialized();

var N: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(TestCases);
  while (cpp_update(TestCases, "--"))
  {
    read(N);
    a = 0;
    b = 0;
    while (((N % 3) == 0))
    {
      N /= 3;
      b += 1;
      a += 1;
    }
    while (((N % 2) == 0))
    {
      N /= 2;
      a -= 1;
    }
    if (((N != 1) || (a < 0)))
    {
      write(-1, "\n");
    } else
    {
      write((a + b), "\n");
    }
  }
  return 0;
}
