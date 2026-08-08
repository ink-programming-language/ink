// Translated from solution.cpp.

func rd(x: dynamic)
{
  x = 0;
  var c = getchar();
  var f = 1;
  while ((!isdigit(c)))
  {
    if ((c == cpp_char("-")))
    {
      f = -1;
    }
    c = getchar();
  }
  while (isdigit(c))
  {
    x = (((x * 10) - cpp_char("0")) + c);
    c = getchar();
  }
  x *= f;
}

var mod = (1e9 + 7);

func Pow(x: dynamic, y: dynamic)
{
  var res = 1;
  {
    while (y)
    {
      if ((y & 1))
      {
        res = ((res * cpp_cast(x)) % mod);
      }
      y >>= 1;
      x = ((x * cpp_cast(x)) % mod);
    }
  }
  return res;
}

var inv100 = Pow(100, (mod - 2));

var f = cpp_array(8);

var mp = cpp_array(8, 8);

var n: dynamic;

var cnt = cpp_array((1 << 7));

var id = cpp_array((1 << 7));

var siz = cpp_array((1 << 7));

var pos = cpp_array((1 << 7), 8);

func main()
{
  rd(n);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          rd(mp[i][j]);
          mp[i][j] = ((mp[i][j] * cpp_cast(inv100)) % mod);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var S = 0;
    while ((S < ((1 << n))))
    {
      cnt[S] = (cnt[(S >> 1)] + ((S & 1)));
      pos[cnt[S]][cpp_assign(id[S], "=", cpp_update(siz[cnt[S]], "++"))] = S;
      S += 1;
    }
  }
  f[0][1] = 1;
  {
    var i = 0;
    while ((i < n))
    {
      var item = cpp_array(8);
      {
        var x = 0;
        while ((x < n))
        {
          {
            var k = 0;
            while ((k < siz[i]))
            {
              if ((!(((pos[i][k] >> x) & 1))))
              {
                item[x].push_back(k);
              }
              k += 1;
            }
          }
          x += 1;
        }
      }
      {
        var it = f[i].begin();
        while ((it != f[i].end()))
        {
          {
            var S = 0;
            while ((S < ((1 << n))))
            {
              var v = it->second;
              var u = 0;
              {
                var x = 0;
                while ((x < n))
                {
                  if (((S >> x) & 1))
                  {
                    v = ((v * cpp_cast(mp[i][x])) % mod);
                    {
                      var t = 0;
                      while ((t < item[x].size()))
                      {
                        if ((((it->first) >> item[x][t]) & 1))
                        {
                          u |= (1 << id[(pos[i][item[x][t]] | (1 << x))]);
                        }
                        t += 1;
                      }
                    }
                  } else
                  {
                    v = ((v * cpp_cast(((1 - mp[i][x])))) % mod);
                  }
                  x += 1;
                }
              }
              if ((!u))
              {
                S += 1;
                continue;
              }
              (cpp_assign(f[(i + 1)][u], "+=", v)) %= mod;
              S += 1;
            }
          }
          it += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", (((f[n][1] + mod)) % mod));
  return 0;
}
