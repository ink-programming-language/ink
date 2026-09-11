// Translated from solution.cpp.

var N: dynamic = (2e6 + 10);

var mod: dynamic = (1e9 + 7);

func rd() -> dynamic
{
  var x: dynamic = 0;
  var w: dynamic = 1;
  var ch: dynamic = 0;
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      w = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = ((x * 10) + ((ch ^ 48)));
    ch = getchar();
  }
  return (x * w);
}

var prm: dynamic = cpp_array(N);

var pm: dynamic = cpp_array(N);

var tt: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var c: dynamic = cpp_array(2, N);

var v: dynamic = cpp_array(N);

func main() -> dynamic
{
  {
    var i: dynamic = 2;
    while ((i <= (N - 5)))
    {
      if ((!pm[i]))
      {
        pm[i] = cpp_assign(prm[cpp_update(tt, "++")], "=", i);
      }
      {
        var j: dynamic = 1;
        while (((i * prm[j]) <= (N - 5)))
        {
          pm[(i * prm[j])] = prm[j];
          if (((i % prm[j]) == 0))
          {
            break;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  n = rd();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i] = rd();
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1));
  reverse((a + 1), ((a + n) + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!c[a[i]][0]))
      {
        v[i] = 1;
        c[a[i]][0] = cpp_assign(c[a[i]][1], "=", 1);
      } else
      {
        var x: dynamic = (a[i] - 1);
        while ((x > 1))
        {
          var np: dynamic = pm[x];
          var cn: dynamic = 0;
          while ((pm[x] == np))
          {
            cn += 1;
            x /= np;
          }
          if ((c[np][0] < cn))
          {
            c[np][0] = cn;
            c[np][1] = 1;
          } else
          {
            c[np][1] += (c[np][0] == cn);
          }
        }
      }
      i += 1;
    }
  }
  var ans: dynamic = 1;
  {
    var i: dynamic = 2;
    while ((i <= (N - 5)))
    {
      {
        var j: dynamic = 1;
        while ((j <= c[i][0]))
        {
          ans = (((1 * ans) * i) % mod);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!v[i]))
      {
        var x: dynamic = (a[i] - 1);
        var o: dynamic = 1;
        while ((x > 1))
        {
          var np: dynamic = pm[x];
          var cn: dynamic = 0;
          while ((pm[x] == np))
          {
            cn += 1;
            x /= np;
          }
          o &= ((cn < c[np][0]) || (c[np][1] > 1));
        }
        if (o)
        {
          ans = (((ans + o)) % mod);
          break;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
