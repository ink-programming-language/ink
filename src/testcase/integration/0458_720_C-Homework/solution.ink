// Translated from solution.cpp.

var N: dynamic = 41414;

var a: dynamic = cpp_array(320, N);

var as_cpp: dynamic = cpp_array(320, N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var ty: dynamic = cpp_uninitialized();

var tl: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var z: dynamic = cpp_uninitialized();

var mx: dynamic = cpp_uninitialized();

func pd(k: dynamic) -> dynamic
{
  if (((((((k < 0) || (k == 1)) || (k == 2)) || (k == 5)) || (k == 4)) || (((k == 8) && (n != 3)) && (m != 3))))
  {
    return 1;
  }
  return 0;
}

func dfs(x: dynamic, y: dynamic, ct: dynamic) -> dynamic
{
  if (tl)
  {
    return;
  }
  if (((((x != 1) || (y != 1))) && (ct == k)))
  {
    tl = 1;
    {
      var i: dynamic = (1);
      while ((i <= (n)))
      {
        {
          var j: dynamic = (1);
          while ((j <= (m)))
          {
            as_cpp[i][j] = a[i][j];
            j += 1;
          }
        }
        i += 1;
      }
    }
    return;
  }
  if ((x > n))
  {
    return;
  }
  if ((y > m))
  {
    return dfs((x + 1), 1, ct);
  }
  if ((cpp_update(z, "++") > mx))
  {
    return;
  }
  t = 0;
  if (a[(x - 1)][y])
  {
    t += ((a[x][(y + 1)] + a[(x - 1)][(y - 1)]) + a[(x - 1)][(y + 1)]);
  }
  if (a[(x + 1)][y])
  {
    t += ((a[x][(y - 1)] + a[(x + 1)][(y - 1)]) + a[(x + 1)][(y + 1)]);
  }
  if (a[x][(y - 1)])
  {
    t += ((a[(x - 1)][y] + a[(x - 1)][(y - 1)]) + a[(x + 1)][(y - 1)]);
  }
  if (a[x][(y + 1)])
  {
    t += ((a[(x - 1)][(y + 1)] + a[(x + 1)][y]) + a[(x + 1)][(y + 1)]);
  }
  if ((((ct + t) <= k) && (((((x < 2) || (y < 2)) || a[(x - 1)][y]) || a[x][(y - 1)]))))
  {
    a[x][y] = 1;
    dfs(x, (y + 1), (ct + t));
    a[x][y] = 0;
  }
  dfs(x, (y + 1), ct);
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    scanf("%d%d%d", (&n), (&m), (&k));
    z = cpp_assign(tl, "=", cpp_assign(ty, "=", 0));
    if ((n < m))
    {
      ty = 1;
      swap(n, m);
    }
    if (((pd((((((n - 1)) * ((m - 1))) * 4) - k)) && (n > 5)) && (m > 5)))
    {
      puts("-1");
      continue;
    }
    mx = ((8 * n) * m);
    dfs(1, 1, 0);
    if ((!tl))
    {
      puts("-1");
    } else if (ty)
    {
      {
        var j: dynamic = (1);
        while ((j <= (m)))
        {
          {
            var i: dynamic = (1);
            while ((i <= (n)))
            {
              putchar( (as_cpp[i][j]) ? cpp_char("*") : cpp_char("."));
              i += 1;
            }
          }
          puts("");
          j += 1;
        }
      }
    } else
    {
      {
        var i: dynamic = (1);
        while ((i <= (n)))
        {
          {
            var j: dynamic = (1);
            while ((j <= (m)))
            {
              putchar( (as_cpp[i][j]) ? cpp_char("*") : cpp_char("."));
              j += 1;
            }
          }
          puts("");
          i += 1;
        }
      }
    }
    puts("");
  }
}
