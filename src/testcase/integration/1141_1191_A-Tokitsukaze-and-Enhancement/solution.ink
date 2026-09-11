// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var x: dynamic = 0;
  if (((n % 4) == 1))
  {
    x = 1;
  } else if (((n % 4) == 3))
  {
    x = 2;
  } else if (((n % 4) == 2))
  {
    x = 3;
  } else
  {
    x = 4;
  }
  if ((x == 1))
  {
    write("0 A");
  } else if ((x == 2))
  {
    write("2 A");
  } else if ((x == 3))
  {
    write("1 B");
  } else
  {
    write("1 A");
  }
  return 0;
}
