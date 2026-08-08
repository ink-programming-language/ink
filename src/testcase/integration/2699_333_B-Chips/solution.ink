// Translated from solution.cpp.

var g = cpp_array(1111, 1111);

func main()
{
  var n: dynamic;
  var m: dynamic;
  memset(g, false, cpp_sizeof((g)));
  scanf("%d%d", (&n), (&m));
  {
    var i = 0;
    while ((i < m))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d%d", (&u), (&v));
      g[u][v] = true;
      i += 1;
    }
  }
  var count = 0;
  {
    var i = 2;
    while ((i < n))
    {
      var flag = false;
      {
        var j = 1;
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
    var j = 2;
    while ((j < n))
    {
      var flag = false;
      {
        var i = 1;
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
    var i: dynamic;
    var j: dynamic;
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
