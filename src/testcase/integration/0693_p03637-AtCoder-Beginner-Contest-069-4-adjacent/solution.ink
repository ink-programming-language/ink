// Translated from solution.cpp.

var n: dynamic;

var s: dynamic;

var x: dynamic;

var y: dynamic;

var z: dynamic;

func main()
{
  read(n);
  {
    var i = 0;
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
