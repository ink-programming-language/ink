// Translated from solution.cpp.

var NS: dynamic = 1111;

var MOD: dynamic = 1000000007;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(11);

var u: dynamic = cpp_array(NS);

var s: dynamic = cpp_array(NS, NS);

var a: dynamic = cpp_array(NS, NS);

func one_num(z: dynamic) -> dynamic
{
  var cnt: dynamic = 0;
  {
    while ((z > 0))
    {
      cnt += 1;
      z = (z & ((z - 1)));
    }
  }
  return cnt;
}

func dfs(x: dynamic, y: dynamic) -> dynamic
{
  if ((y > m))
  {
    return dfs((x + 1), 1);
  }
  if ((x > n))
  {
    return 1;
  }
  var cur: dynamic = (s[(x - 1)][y] | s[x][(y - 1)]);
  if (((((n + m) - x) - y) >= (k - builtin_popcount(cur))))
  {
    return 0;
  }
  var ans: dynamic = 0;
  var tmp: dynamic = -1;
  {
    var t: dynamic = ((((~cur)) & ((((1 << k)) - 1))));
    while ((t > 0))
    {
      var i: dynamic = (u[(t & ((-t)))] + 1);
      if (((!((cur & ((1 << ((i - 1))))))) && (((a[x][y] == i) || (!a[x][y])))))
      {
        s[x][y] = (cur | ((1 << ((i - 1)))));
        if (v[i])
        {
          v[i] += 1;
          ans += dfs(x, (y + 1));
        } else
        {
          v[i] += 1;
          if ((tmp == -1))
          {
            tmp = dfs(x, (y + 1));
          }
          ans += tmp;
        }
        v[i] -= 1;
        ans %= MOD;
      }
      t -= (t & ((-t)));
    }
  }
  return ans;
}

func main() -> dynamic
{
  while ((~scanf("%d%d%d", (&n), (&m), (&k))))
  {
    memset(v, 0, cpp_sizeof((v)));
    {
      var i: dynamic = 0;
      while ((i <= k))
      {
        u[(1 << i)] = i;
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
            scanf("%d", (&a[i][j]));
            v[a[i][j]] += 1;
            j += 1;
          }
        }
        i += 1;
      }
    }
    if (((((n + m) - 1)) > k))
    {
      printf("0\n");
      continue;
    }
    var ans: dynamic = dfs(1, 1);
    printf("%I64d\n", ans);
  }
  return 0;
}
