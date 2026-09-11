// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  if ((n == m))
  {
    write((n + 1), "\n");
    {
      i = 0;
      while ((i <= m))
      {
        printf("%d %d\n", (m - i), i);
        i += 1;
      }
    }
  } else if ((n < m))
  {
    write((n + 1), "\n");
    {
      i = 0;
      while ((i <= n))
      {
        printf("%d %d\n", i, (i + 1));
        i += 1;
      }
    }
  } else
  {
    write((m + 1), "\n");
    {
      i = 0;
      while ((i <= m))
      {
        printf("%d %d\n", (i + 1), i);
        i += 1;
      }
    }
  }
  return 0;
}
