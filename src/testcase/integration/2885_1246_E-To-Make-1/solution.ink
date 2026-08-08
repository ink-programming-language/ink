// Translated from solution.cpp.

var dp = cpp_array((1 << 16));

var n: dynamic;

var k: dynamic;

var a = cpp_array(16);

var weight = cpp_array(16);

func calc(x: dynamic)
{
  var c = 0;
  while (((x % k) == 0))
  {
    x /= k;
    c += 1;
  }
  return c;
}

func calc(x: dynamic)
{
  var c = 0;
  while (((x % k) == 0))
  {
    x /= k;
    c += 1;
  }
  return c;
}

func dfs(s: dynamic, x: dynamic)
{
  if ((builtin_popcount(s) == 1))
  {
    return;
  }
  var y = x;
  var z = x;
  while ((y < (16 * 2002)))
  {
    if (dp[s][y])
    {
      z = y;
    }
    y = (y * k);
  }
  if ((z != x))
  {
    {
      var i = 0;
      while ((i < n))
      {
        if ((((1 << i)) & s))
        {
          weight[i] += calc((z / x));
        }
        i += 1;
      }
    }
    dfs(s, z);
  } else
  {
    {
      var i = 0;
      while ((i < n))
      {
        if (cpp_binary(cpp_binary((((1 << i)) & s), "and", (x >= a[i])), "and", dp[(s ^ ((1 << i)))][(x - a[i])]))
        {
          dfs((s ^ ((1 << i))), (x - a[i]));
          break;
        }
        i += 1;
      }
    }
  }
}

func main()
{
  dp[0][0] = 1;
  scanf("%d%d", (&n), (&k));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < ((1 << n))))
    {
      {
        var j = 0;
        while ((j < n))
        {
          if ((i & ((1 << j))))
          {
            dp[i] |= ((dp[(i - ((1 << j)))] << a[j]));
          }
          j += 1;
        }
      }
      {
        var j = ((16 * 2000) / k);
        while ((j >= 0))
        {
          if (dp[i][(j * k)])
          {
            dp[i][j] = 1;
          }
          j -= 1;
        }
      }
      i += 1;
    }
  }
  if ((!dp[(((1 << n)) - 1)][1]))
  {
    return (puts("NO") * 0);
  }
  puts("YES");
  dfs((((1 << n)) - 1), 1);
  var cnt = (n - 1);
  while (cpp_update(cnt, "--"))
  {
    var f1 = -1;
    var f2 = -1;
    {
      var i = 0;
      while ((i < n))
      {
        if (cpp_binary((f1 == -1), "or", (weight[i] > weight[f1])))
        {
          f2 = f1;
          f1 = i;
        } else if (cpp_binary((f2 == -1), "or", (weight[i] > weight[f2])))
        {
          f2 = i;
        }
        i += 1;
      }
    }
    printf("%d %d\n", a[f1], a[f2]);
    a[f1] += a[f2];
    weight[f2] = -1e9;
    weight[f1] -= calc(a[f1]);
  }
  return 0;
}
