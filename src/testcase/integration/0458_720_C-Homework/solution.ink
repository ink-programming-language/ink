// Translated from solution.cpp.

var N = 41414;

var a = cpp_array(320, N);

var as_cpp = cpp_array(320, N);

var n: dynamic;

var m: dynamic;

var k: dynamic;

var ty: dynamic;

var tl: dynamic;

var t: dynamic;

var z: dynamic;

var mx: dynamic;

func pd(k: dynamic)
{
  if (((((((k < 0) || (k == 1)) || (k == 2)) || (k == 5)) || (k == 4)) || (((k == 8) && (n != 3)) && (m != 3))))
  {
    return 1;
  }
  return 0;
}

func dfs(x: dynamic, y: dynamic, ct: dynamic)
{
  if (tl)
  {
    return;
  }
  if (((((x != 1) || (y != 1))) && (ct == k)))
  {
    tl = 1;
    {
      var i = (1);
      while ((i <= (n)))
      {
        {
          var j = (1);
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

func main()
{
  var t: dynamic;
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
        var j = (1);
        while ((j <= (m)))
        {
          {
            var i = (1);
            while ((i <= (n)))
            {
              putchar(if (as_cpp[i][j]) cpp_char("*") else cpp_char("."));
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
        var i = (1);
        while ((i <= (n)))
        {
          {
            var j = (1);
            while ((j <= (m)))
            {
              putchar(if (as_cpp[i][j]) cpp_char("*") else cpp_char("."));
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
