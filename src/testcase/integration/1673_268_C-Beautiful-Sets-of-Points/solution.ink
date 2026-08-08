// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  var j: dynamic;
  var i: dynamic;
  var k: dynamic;
  var n: dynamic;
  var m: dynamic;
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
