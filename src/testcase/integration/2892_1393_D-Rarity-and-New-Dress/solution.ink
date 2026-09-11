// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var c: dynamic = cpp_array(2005, 2005);

var up: dynamic = cpp_array(2005, 2005);

var down: dynamic = cpp_array(2005, 2005);

var le: dynamic = cpp_array(2005, 2005);

var ri: dynamic = cpp_array(2005, 2005);

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%s", (c[i] + 1));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          up[i][j] =  (((c[i][j] == c[(i - 1)][j]))) ? (up[(i - 1)][j] + 1) : 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = n;
    while (i)
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          down[i][j] =  (((c[i][j] == c[(i + 1)][j]))) ? (down[(i + 1)][j] + 1) : 1;
          j += 1;
        }
      }
      i -= 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
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
        var j: dynamic = m;
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
        var j: dynamic = 1;
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
