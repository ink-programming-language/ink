// Translated from solution.cpp.

var N = (1e6 + 10);

var mod = (1e9 + 7);

var mod2 = 998244353;

var inf = 8e18;

var LOG = 22;

func pw(a: dynamic, b: dynamic, M: dynamic)
{
  return (if ((!b)) 1 else (if ((b & 1)) (((a * pw(((a * a) % M), (b / 2), M))) % M) else pw(((a * a) % M), (b / 2), M)));
}

var C = cpp_array(N);

var sum = cpp_array(N);

var ans = cpp_array(N);

var n: dynamic;

var A = cpp_array(N);

var hide = cpp_array(N);

var sub = cpp_array(N);

var mask = cpp_array(N);

var T = cpp_array(((1 << LOG)));

var G = cpp_array(N);

var vec: dynamic;

func dfs(v: dynamic, P: dynamic)
{
  sub[v] = 1;
  for (var u in G[v])
  {
    if ((hide[u] || (u == P)))
    {
      continue;
    }
    dfs(u, v);
    sub[v] += sub[u];
  }
}

func find(v: dynamic, P: dynamic, n: dynamic)
{
  for (var u in G[v])
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

func pre(v: dynamic, P: dynamic)
{
  sum[v] = 0;
  mask[v] = (A[v] ^ mask[P]);
  T[mask[v]] += 1;
  for (var u in G[v])
  {
    if ((hide[u] || (u == P)))
    {
      continue;
    }
    pre(u, v);
  }
}

func clear(v: dynamic, P: dynamic)
{
  sum[v] = 0;
  T[mask[v]] -= 1;
  for (var u in G[v])
  {
    if ((hide[u] || (u == P)))
    {
      continue;
    }
    clear(u, v);
  }
}

func erase(v: dynamic, P: dynamic)
{
  vec.push_back(v);
  for (var u in G[v])
  {
    if ((hide[u] || (u == P)))
    {
      continue;
    }
    erase(u, v);
  }
}

func calc(v: dynamic, P: dynamic)
{
  for (var u in G[v])
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

func dec(v: dynamic)
{
  dfs(v, 0);
  var n = sub[v];
  var centroid = find(v, 0, n);
  pre(centroid, 0);
  hide[centroid] = 1;
  T[mask[centroid]] -= 1;
  sum[centroid] += T[0];
  {
    var i = 0;
    while ((i < LOG))
    {
      var mask2 = ((1 << i));
      sum[centroid] += T[mask2];
      i += 1;
    }
  }
  T[mask[centroid]] += 1;
  for (var u in G[centroid])
  {
    if (hide[u])
    {
      continue;
    }
    vec.clear();
    erase(u, 0);
    for (var x in vec)
    {
      T[mask[x]] -= 1;
    }
    for (var x in vec)
    {
      mask[x] ^= mask[centroid];
      sum[x] += T[mask[x]];
      {
        var i = 0;
        while ((i < LOG))
        {
          var mask2 = (mask[x] ^ ((1 << i)));
          sum[x] += T[mask2];
          i += 1;
        }
      }
      mask[x] ^= mask[centroid];
    }
    for (var x in vec)
    {
      T[mask[x]] += 1;
    }
  }
  calc(centroid, 0);
  ans[centroid] -= (sum[centroid] / 2);
  clear(centroid, 0);
  for (var u in G[centroid])
  {
    if (hide[u])
    {
      continue;
    }
    dec(u);
  }
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i < n))
    {
      var a: dynamic;
      var b: dynamic;
      scanf("%d%d", (&a), (&b));
      G[a].push_back(b);
      G[b].push_back(a);
      i += 1;
    }
  }
  scanf("%s", C);
  {
    var i = 0;
    while ((i < n))
    {
      A[(i + 1)] = (1 << ((C[i] - cpp_char("a"))));
      i += 1;
    }
  }
  dec(1);
  {
    var i = 1;
    while ((i <= n))
    {
      printf("%lld ", (ans[i] + 1));
      i += 1;
    }
  }
  return 0;
}
