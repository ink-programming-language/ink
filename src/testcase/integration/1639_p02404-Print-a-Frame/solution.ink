// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = 5;
  while (1)
  {
    read(a, b);
    if (((a == 0) && (b == 0)))
    {
      break;
    }
    {
      var i: dynamic = 0;
      while ((i < a))
      {
        {
          var j: dynamic = 0;
          while ((j < b))
          {
            if (((((i == 0) || (j == 0)) || (i == (a - 1))) || (j == (b - 1))))
            {
              write("#");
            } else
            {
              write(".");
            }
            j += 1;
          }
        }
        write("\n");
        i += 1;
      }
    }
    write("\n");
  }
  return 0;
}
