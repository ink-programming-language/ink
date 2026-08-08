// Translated from solution.cpp.

func SET(n: dynamic, pos: dynamic)
{
  return cpp_assign(n, "=", (n | ((1 << pos))));
}

func RESET(n: dynamic, pos: dynamic)
{
  return cpp_assign(n, "=", (n & (~((1 << pos)))));
}

func CHECK(n: dynamic, pos: dynamic)
{
  return cpp_cast(((n & ((1 << pos)))));
}

func bigMod(n: dynamic, power: dynamic, MOD: dynamic)
{
  if ((power == 0))
  {
    return 1;
  }
  if (((power % 2) == 0))
  {
    var ret = bigMod(n, (power / 2), MOD);
    return (((((ret % MOD)) * ((ret % MOD)))) % MOD);
  } else
  {
    return (((((n % MOD)) * ((bigMod(n, (power - 1), MOD) % MOD)))) % MOD);
  }
}

func modInverse(n: dynamic, MOD: dynamic)
{
  return bigMod(n, (MOD - 2), MOD);
}

func POW(x: dynamic, y: dynamic)
{
  var res = 1;
  {
    while (y)
    {
      if (((y & 1)))
      {
        res *= x;
      }
      x *= x;
      y >>= 1;
    }
  }
  return res;
}

func inverse(x: dynamic)
{
  var p = ((cpp_cast(1.0)) / x);
  return ((p) + 1e-9);
}

func gcd(a: dynamic, b: dynamic)
{
  while (b)
  {
    b ^= cpp_assign(a, "^=", cpp_assign(b, "^=", cpp_assign(a, "%=", b)));
  }
  return a;
}

func nC2(n: dynamic)
{
  return ((n * ((n - 1))) / 2);
}

func MOD(n: dynamic, mod: dynamic)
{
  if ((n >= 0))
  {
    return (n % mod);
  } else if (((-n) == mod))
  {
    return 0;
  } else
  {
    return (mod + ((n % mod)));
  }
}

var vec = cpp_array(1000001);

var ans = cpp_array(1000001);

var vis = cpp_array(1000001);

var ara = cpp_array(1000001);

var ind = cpp_array(1000001);

var n: dynamic;

var m: dynamic;

var ok = true;

func foo(u: dynamic, num: dynamic)
{
  vis[u] = 1;
  {
    var i = 0;
    while ((i < vec[u].size()))
    {
      var v = vec[u][i];
      if ((!vis[v]))
      {
        ans[num].push_back(ara[v]);
        ind[v] = num;
        foo(v, num);
      }
      i += 1;
    }
  }
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      read(ara[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      vec[a].push_back(b);
      vec[b].push_back(a);
      i += 1;
    }
  }
  var level = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!vis[i]))
      {
        ans[level].push_back(ara[i]);
        ind[i] = level;
        foo(i, level);
        level += 1;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      sort(ans[i].begin(), ans[i].end());
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      write(ans[ind[i]].back(), " ");
      ans[ind[i]].pop_back();
      i += 1;
    }
  }
  return 0;
}
