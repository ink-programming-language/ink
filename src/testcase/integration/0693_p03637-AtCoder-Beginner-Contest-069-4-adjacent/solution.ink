// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var z: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(s);
      if (((s % 4) == 0))
      {
        x += 1;
      } else if (((s % 2) == 0))
      {
        y += 1;
      } else
      {
        z += 1;
      }
      i += 1;
    }
  }
  if ((x >= z))
  {
    write("Yes", "\n");
  } else if (cpp_binary(((z - x) == 1), "and", (y == 0)))
  {
    write("Yes", "\n");
  } else
  {
    write("No", "\n");
  }
}
