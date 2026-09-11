// Translated from solution.cpp.

var N: dynamic = (1e6 + 10);

var mod: dynamic = (1e9 + 7);

var mod2: dynamic = 998244353;

var inf: dynamic = 8e18;

var LOG: dynamic = 22;

func pw(a: dynamic, b: dynamic, M: dynamic) -> dynamic
{
  return ( ((!b)) ? 1 : ( ((b & 1)) ? (((a * pw(((a * a) % M), (b / 2), M))) % M) : pw(((a * a) % M), (b / 2), M)));
}

var C: dynamic = cpp_array(N);

var sum: dynamic = cpp_array(N);

var ans: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array(N);

var hide: dynamic = cpp_array(N);

var sub: dynamic = cpp_array(N);

var mask: dynamic = cpp_array(N);

var T: dynamic = cpp_array(((1 << LOG)));

var G: dynamic = cpp_array(N);

var vec: dynamic = cpp_uninitialized();

func dfs(v: dynamic, P: dynamic) -> dynamic
{
  sub[v] = 1;
  for (var u: dynamic in G[v])
  {
    if ((hide[u] || (u == P)))
    {
      continue;
    }
    dfs(u, v);
    sub[v] += sub[u];
  }
}

func find(v: dynamic, P: dynamic, n: dynamic) -> dynamic
{
  for (var u: dynamic in G[v])
  {
    if ((hide[u] || (u == P)))
    {
      continue;
    }
    if (((sub[u] * 2) > n))
    {
      return find(u, v, n);
    }
  }
  return v;
}

func pre(v: dynamic, P: dynamic) -> dynamic
{
  sum[v] = 0;
  mask[v] = (A[v] ^ mask[P]);
  T[mask[v]] += 1;
  for (var u: dynamic in G[v])
  {
    if ((hide[u] || (u == P)))
    {
      continue;
    }
    pre(u, v);
  }
}

func clear(v: dynamic, P: dynamic) -> dynamic
{
  sum[v] = 0;
  T[mask[v]] -= 1;
  for (var u: dynamic in G[v])
  {
    if ((hide[u] || (u == P)))
    {
      continue;
    }
    clear(u, v);
  }
}

func erase(v: dynamic, P: dynamic) -> dynamic
{
  vec.push_back(v);
  for (var u: dynamic in G[v])
  {
    if ((hide[u] || (u == P)))
    {
      continue;
    }
    erase(u, v);
  }
}

func calc(v: dynamic, P: dynamic) -> dynamic
{
  for (var u: dynamic in G[v])
  {
    if ((hide[u] || (u == P)))
    {
      continue;
    }
    calc(u, v);
    sum[v] += sum[u];
  }
  ans[v] += sum[v];
}

func dec(v: dynamic) -> dynamic
{
  dfs(v, 0);
  var n: dynamic = sub[v];
  var centroid: dynamic = find(v, 0, n);
  pre(centroid, 0);
  hide[centroid] = 1;
  T[mask[centroid]] -= 1;
  sum[centroid] += T[0];
  {
    var i: dynamic = 0;
    while ((i < LOG))
    {
      var mask2: dynamic = ((1 << i));
      sum[centroid] += T[mask2];
      i += 1;
    }
  }
  T[mask[centroid]] += 1;
  for (var u: dynamic in G[centroid])
  {
    if (hide[u])
    {
      continue;
    }
    vec.clear();
    erase(u, 0);
    for (var x: dynamic in vec)
    {
      T[mask[x]] -= 1;
    }
    for (var x: dynamic in vec)
    {
      mask[x] ^= mask[centroid];
      sum[x] += T[mask[x]];
      {
        var i: dynamic = 0;
        while ((i < LOG))
        {
          var mask2: dynamic = (mask[x] ^ ((1 << i)));
          sum[x] += T[mask2];
          i += 1;
        }
      }
      mask[x] ^= mask[centroid];
    }
    for (var x: dynamic in vec)
    {
      T[mask[x]] += 1;
    }
  }
  calc(centroid, 0);
  ans[centroid] -= (sum[centroid] / 2);
  clear(centroid, 0);
  for (var u: dynamic in G[centroid])
  {
    if (hide[u])
    {
      continue;
    }
    dec(u);
  }
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      scanf("%d%d", (&a), (&b));
      G[a].push_back(b);
      G[b].push_back(a);
      i += 1;
    }
  }
  scanf("%s", C);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      A[(i + 1)] = (1 << ((C[i] - cpp_char("a"))));
      i += 1;
    }
  }
  dec(1);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      printf("%lld ", (ans[i] + 1));
      i += 1;
    }
  }
  return 0;
}
