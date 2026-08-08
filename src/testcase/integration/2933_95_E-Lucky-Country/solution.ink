// Translated from solution.cpp.

func sc(x: dynamic)
{
  var c = getchar();
  x = 0;
  var neg = 0;
  {
    while ((((((c < 48) | (c > 57))) && (c != cpp_char("-")))))
    {
      c = getchar();
    }
  }
  if ((c == cpp_char("-")))
  {
    neg = 1;
    c = getchar();
  }
  {
    while (((c > 47) && (c < 58)))
    {
      x = (((((x << 1)) + ((x << 3))) + c) - 48);
      c = getchar();
    }
  }
  if (neg)
  {
    x = (-x);
  }
}

func bigmod(p: dynamic, e: dynamic, M: dynamic)
{
  var ret = 1;
  {
    while ((e > 0))
    {
      if ((e & 1))
      {
        ret = (((ret * p)) % M);
      }
      p = (((p * p)) % M);
      e >>= 1;
    }
  }
  return cpp_cast(ret);
}

func gcd(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func modinverse(a: dynamic, M: dynamic)
{
  return bigmod(a, (M - 2), M);
}

var N = (1e5 + 55);

var L: dynamic;

var qq: dynamic;

var dp = cpp_array(N);

var coin: dynamic;

var vis = cpp_array(N);

var G = cpp_array(N);

var us = cpp_array(N);

func dfs(u: dynamic)
{
  if ((u > N))
  {
    return;
  }
  if ((u > 0))
  {
    L.push_back((u));
  }
  dfs(((10 * u) + 4));
  dfs(((10 * u) + 7));
}

func dfs(u: dynamic, p: dynamic)
{
  vis[u] = 1;
  var ret = 0;
  for (var a in G[u])
  {
    if (((a != p) && (vis[a] == 0)))
    {
      ret += dfs(a, u);
    }
  }
  return (1 + ret);
}

func main()
{
  dfs(0);
  var n: dynamic;
  var m: dynamic;
  sc(n);
  sc(m);
  {
    var i = 0;
    while ((i < m))
    {
      var a: dynamic;
      var b: dynamic;
      sc(a);
      sc(b);
      G[a].push_back((b));
      G[b].push_back((a));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((vis[i] == 0))
      {
        var t = dfs(i, -1);
        qq[t] += 1;
      }
      i += 1;
    }
  }
  for (var a in qq)
  {
    var x = a.first;
    var y = a.second;
    {
      var i = 1;
      while ((i <= y))
      {
        coin.push_back((make_pair((x * i), i)));
        y -= i;
        i *= 2;
      }
    }
    if ((y > 0))
    {
      coin.push_back((make_pair((x * y), y)));
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      dp[i] = (1 << 30);
      i += 1;
    }
  }
  dp[0] = 0;
  var tot = 0;
  for (var a in coin)
  {
    tot += a.first;
    {
      var j = tot;
      while ((j >= a.first))
      {
        dp[j] = (if (((dp[j]) > ((dp[(j - a.first)] + a.second)))) ((dp[(j - a.first)] + a.second)) else (dp[j]));
        j -= 1;
      }
    }
  }
  var iM = (1 << 30);
  for (var a in L)
  {
    iM = (if (((iM) > (dp[a]))) (dp[a]) else (iM));
  }
  printf("%d\n", if ((iM == (1 << 30))) -1 else (iM - 1));
}
