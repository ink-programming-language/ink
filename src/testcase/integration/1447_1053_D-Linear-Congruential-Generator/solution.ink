// Translated from solution.cpp.

var N = (2e6 + 10);

var mod = (1e9 + 7);

func rd()
{
  var x = 0;
  var w = 1;
  var ch = 0;
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

var prm = cpp_array(N);

var pm = cpp_array(N);

var tt: dynamic;

var n: dynamic;

var a = cpp_array(N);

var c = cpp_array(2, N);

var v = cpp_array(N);

func main()
{
  {
    var i = 2;
    while ((i <= (N - 5)))
    {
      if ((!pm[i]))
      {
        pm[i] = cpp_assign(prm[cpp_update(tt, "++")], "=", i);
      }
      {
        var j = 1;
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
    var i = 1;
    while ((i <= n))
    {
      a[i] = rd();
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1));
  reverse((a + 1), ((a + n) + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!c[a[i]][0]))
      {
        v[i] = 1;
        c[a[i]][0] = cpp_assign(c[a[i]][1], "=", 1);
      } else
      {
        var x = (a[i] - 1);
        while ((x > 1))
        {
          var np = pm[x];
          var cn = 0;
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
  var ans = 1;
  {
    var i = 2;
    while ((i <= (N - 5)))
    {
      {
        var j = 1;
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
    var i = 1;
    while ((i <= n))
    {
      if ((!v[i]))
      {
        var x = (a[i] - 1);
        var o = 1;
        while ((x > 1))
        {
          var np = pm[x];
          var cn = 0;
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
