// Translated from solution.cpp.

func SET(n: dynamic, pos: dynamic) -> dynamic
{
  return cpp_assign(n, "=", (n | ((1 << pos))));
}

func RESET(n: dynamic, pos: dynamic) -> dynamic
{
  return cpp_assign(n, "=", (n & (~((1 << pos)))));
}

func CHECK(n: dynamic, pos: dynamic) -> dynamic
{
  return cpp_cast(((n & ((1 << pos)))));
}

func bigMod(n: dynamic, power: dynamic, MOD: dynamic) -> dynamic
{
  if ((power == 0))
  {
    return 1;
  }
  if (((power % 2) == 0))
  {
    var ret: dynamic = bigMod(n, (power / 2), MOD);
    return (((((ret % MOD)) * ((ret % MOD)))) % MOD);
  } else
  {
    return (((((n % MOD)) * ((bigMod(n, (power - 1), MOD) % MOD)))) % MOD);
  }
}

func modInverse(n: dynamic, MOD: dynamic) -> dynamic
{
  return bigMod(n, (MOD - 2), MOD);
}

func POW(x: dynamic, y: dynamic) -> dynamic
{
  var res: dynamic = 1;
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

func inverse(x: dynamic) -> dynamic
{
  var p: dynamic = ((cpp_cast(1.0)) / x);
  return ((p) + 1e-9);
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  while (b)
  {
    b ^= cpp_assign(a, "^=", cpp_assign(b, "^=", cpp_assign(a, "%=", b)));
  }
  return a;
}

func nC2(n: dynamic) -> dynamic
{
  return ((n * ((n - 1))) / 2);
}

func MOD(n: dynamic, mod: dynamic) -> dynamic
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

var vec: dynamic = cpp_array(1000001);

var ans: dynamic = cpp_array(1000001);

var vis: dynamic = cpp_array(1000001);

var ara: dynamic = cpp_array(1000001);

var ind: dynamic = cpp_array(1000001);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ok: dynamic = true;

func foo(u: dynamic, num: dynamic) -> dynamic
{
  vis[u] = 1;
  {
    var i: dynamic = 0;
    while ((i < vec[u].size()))
    {
      var v: dynamic = vec[u][i];
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(ara[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b);
      vec[a].push_back(b);
      vec[b].push_back(a);
      i += 1;
    }
  }
  var level: dynamic = 1;
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      sort(ans[i].begin(), ans[i].end());
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      write(ans[ind[i]].back(), " ");
      ans[ind[i]].pop_back();
      i += 1;
    }
  }
  return 0;
}
