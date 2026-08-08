// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var ans: dynamic;

var c = cpp_array(2005, 2005);

var up = cpp_array(2005, 2005);

var down = cpp_array(2005, 2005);

var le = cpp_array(2005, 2005);

var ri = cpp_array(2005, 2005);

func main()
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%s", (c[i] + 1));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= m))
        {
          up[i][j] = if (((c[i][j] == c[(i - 1)][j]))) (up[(i - 1)][j] + 1) else 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = n;
    while (i)
    {
      {
        var j = 1;
        while ((j <= m))
        {
          down[i][j] = if (((c[i][j] == c[(i + 1)][j]))) (down[(i + 1)][j] + 1) else 1;
          j += 1;
        }
      }
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= m))
        {
          if ((c[i][j] != c[i][(j - 1)]))
          {
            le[i][j] = 1;
          } else
          {
            le[i][j] = min(min(up[i][j], down[i][j]), (le[i][(j - 1)] + 1));
          }
          j += 1;
        }
      }
      {
        var j = m;
        while ((j >= 1))
        {
          if ((c[i][j] != c[i][(j + 1)]))
          {
            ri[i][j] = 1;
          } else
          {
            ri[i][j] = min(min(up[i][j], down[i][j]), (ri[i][(j + 1)] + 1));
          }
          j -= 1;
        }
      }
      {
        var j = 1;
        while ((j <= m))
        {
          ans += min(le[i][j], ri[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%lld", ans);
}
