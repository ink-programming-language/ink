// Translated from solution.cpp.

func rd(x: dynamic) -> dynamic
{
  x = 0;
  var c: dynamic = getchar();
  var f: dynamic = 1;
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

var mod: dynamic = (1e9 + 7);

func Pow(x: dynamic, y: dynamic) -> dynamic
{
  var res: dynamic = 1;
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

var inv100: dynamic = Pow(100, (mod - 2));

var f: dynamic = cpp_array(8);

var mp: dynamic = cpp_array(8, 8);

var n: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_array((1 << 7));

var id: dynamic = cpp_array((1 << 7));

var siz: dynamic = cpp_array((1 << 7));

var pos: dynamic = cpp_array((1 << 7), 8);

func main() -> dynamic
{
  rd(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
    var S: dynamic = 0;
    while ((S < ((1 << n))))
    {
      cnt[S] = (cnt[(S >> 1)] + ((S & 1)));
      pos[cnt[S]][cpp_assign(id[S], "=", cpp_update(siz[cnt[S]], "++"))] = S;
      S += 1;
    }
  }
  f[0][1] = 1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var item: dynamic = cpp_array(8);
      {
        var x: dynamic = 0;
        while ((x < n))
        {
          {
            var k: dynamic = 0;
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
        var it: dynamic = f[i].begin();
        while ((it != f[i].end()))
        {
          {
            var S: dynamic = 0;
            while ((S < ((1 << n))))
            {
              var v: dynamic = it->second;
              var u: dynamic = 0;
              {
                var x: dynamic = 0;
                while ((x < n))
                {
                  if (((S >> x) & 1))
                  {
                    v = ((v * cpp_cast(mp[i][x])) % mod);
                    {
                      var t: dynamic = 0;
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
