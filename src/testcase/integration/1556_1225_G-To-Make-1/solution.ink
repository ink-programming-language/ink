// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(16);

var b: dynamic = cpp_array(16);

var f: dynamic = cpp_array((1 << 16));

var vis: dynamic = cpp_array(16);

func solve(s: dynamic, i: dynamic) -> dynamic
{
  if ((!((s & ((s - 1))))))
  {
    return;
  }
  {
    var u: dynamic = 0;
    while ((u <= (n - 1)))
    {
      if (((((s >> u) & 1) && (a[u] < i)) && f[(s ^ ((1 << u)))][(i - a[u])]))
      {
        solve((s ^ ((1 << u))), (i - a[u]));
        return;
      }
      u += 1;
    }
  }
  solve(s, (i * k));
  {
    var u: dynamic = 0;
    while ((u <= (n - 1)))
    {
      if (((s >> u) & 1))
      {
        b[u] += 1;
      }
      u += 1;
    }
  }
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&k));
  {
    var i: dynamic = 0;
    while ((i <= (n - 1)))
    {
      scanf("%d", (a + i));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= (n - 1)))
    {
      f[(1 << i)][a[i]] = 1;
      i += 1;
    }
  }
  {
    var s: dynamic = 1;
    while ((s <= (((1 << n)) - 1)))
    {
      if ((s & ((s - 1))))
      {
        var z: dynamic = 0;
        {
          var u: dynamic = 0;
          while ((u <= (n - 1)))
          {
            if (((s >> u) & 1))
            {
              z += a[u];
              f[s] |= (f[(s ^ ((1 << u)))] << a[u]);
            }
            u += 1;
          }
        }
        {
          var i: dynamic = (z / k);
          while ((i >= 1))
          {
            if (f[s][(i * k)])
            {
              f[s][i] = 1;
            }
            i -= 1;
          }
        }
      }
      s += 1;
    }
  }
  if ((!f[(((1 << n)) - 1)][1]))
  {
    return cpp_comma(puts("NO"), 0);
  }
  puts("YES");
  solve((((1 << n)) - 1), 1);
  {
    var T: dynamic = 1;
    while ((T <= (n - 1)))
    {
      var mx: dynamic = 0;
      {
        var i: dynamic = 0;
        while ((i <= (n - 1)))
        {
          if ((!vis[i]))
          {
            mx = max(mx, b[i]);
          }
          i += 1;
        }
      }
      var x: dynamic = -1;
      var y: dynamic = -1;
      {
        var i: dynamic = 0;
        while ((i <= (n - 1)))
        {
          if (((!vis[i]) && (mx == b[i])))
          {
            if ((!(~x)))
            {
              x = i;
            } else if ((!(~y)))
            {
              y = i;
            }
          }
          i += 1;
        }
      }
      printf("%d %d\n", a[x], a[y]);
      vis[y] = 1;
      a[x] += a[y];
      while (((a[x] % k) == 0))
      {
        a[x] /= k;
        b[x] -= 1;
      }
      T += 1;
    }
  }
  return 0;
}
