// Translated from solution.cpp.

var TestCases: dynamic;

var N: dynamic;

var a: dynamic;

var b: dynamic;

func main()
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
