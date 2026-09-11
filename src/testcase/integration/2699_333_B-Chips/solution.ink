// Translated from solution.cpp.

var g: dynamic = cpp_array(1111, 1111);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  memset(g, false, cpp_sizeof((g)));
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d%d", (&u), (&v));
      g[u][v] = true;
      i += 1;
    }
  }
  var count: dynamic = 0;
  {
    var i: dynamic = 2;
    while ((i < n))
    {
      var flag: dynamic = false;
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          if ((g[i][j] == true))
          {
            flag = true;
            break;
          }
          j += 1;
        }
      }
      if (flag)
      {
        i += 1;
        continue;
      }
      count += 1;
      i += 1;
    }
  }
  {
    var j: dynamic = 2;
    while ((j < n))
    {
      var flag: dynamic = false;
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          if (g[i][j])
          {
            flag = true;
            break;
          }
          i += 1;
        }
      }
      if (flag)
      {
        j += 1;
        continue;
      }
      count += 1;
      j += 1;
    }
  }
  if ((n & 1))
  {
    var i: dynamic = cpp_uninitialized();
    var j: dynamic = cpp_uninitialized();
    {
      i = 1;
      while ((i <= n))
      {
        if (g[i][((n / 2) + 1)])
        {
          break;
        }
        i += 1;
      }
    }
    {
      j = 1;
      while ((j <= n))
      {
        if (g[((n / 2) + 1)][j])
        {
          break;
        }
        j += 1;
      }
    }
    if (((i > n) && (j > n)))
    {
      if (count)
      {
        count -= 1;
      }
    }
  }
  printf("%d\n", count);
  return 0;
}
