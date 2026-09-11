// Translated from solution.cpp.

func sc(x: dynamic) -> dynamic
{
  var c: dynamic = getchar();
  x = 0;
  var neg: dynamic = 0;
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

func bigmod(p: dynamic, e: dynamic, M: dynamic) -> dynamic
{
  var ret: dynamic = 1;
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

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func modinverse(a: dynamic, M: dynamic) -> dynamic
{
  return bigmod(a, (M - 2), M);
}

var N: dynamic = (1e5 + 55);

var L: dynamic = cpp_uninitialized();

var qq: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(N);

var coin: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(N);

var G: dynamic = cpp_array(N);

var us: dynamic = cpp_array(N);

func dfs(u: dynamic) -> dynamic
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

func dfs(u: dynamic, p: dynamic) -> dynamic
{
  vis[u] = 1;
  var ret: dynamic = 0;
  for (var a: dynamic in G[u])
  {
    if (((a != p) && (vis[a] == 0)))
    {
      ret += dfs(a, u);
    }
  }
  return (1 + ret);
}

func main() -> dynamic
{
  dfs(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  sc(n);
  sc(m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      sc(a);
      sc(b);
      G[a].push_back((b));
      G[b].push_back((a));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((vis[i] == 0))
      {
        var t: dynamic = dfs(i, -1);
        qq[t] += 1;
      }
      i += 1;
    }
  }
  for (var a: dynamic in qq)
  {
    var x: dynamic = a.first;
    var y: dynamic = a.second;
    {
      var i: dynamic = 1;
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
    var i: dynamic = 0;
    while ((i < N))
    {
      dp[i] = (1 << 30);
      i += 1;
    }
  }
  dp[0] = 0;
  var tot: dynamic = 0;
  for (var a: dynamic in coin)
  {
    tot += a.first;
    {
      var j: dynamic = tot;
      while ((j >= a.first))
      {
        dp[j] = ( (((dp[j]) > ((dp[(j - a.first)] + a.second)))) ? ((dp[(j - a.first)] + a.second)) : (dp[j]));
        j -= 1;
      }
    }
  }
  var iM: dynamic = (1 << 30);
  for (var a: dynamic in L)
  {
    iM = ( (((iM) > (dp[a]))) ? (dp[a]) : (iM));
  }
  printf("%d\n",  ((iM == (1 << 30))) ? -1 : (iM - 1));
}
