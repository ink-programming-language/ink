// Translated from solution.cpp.

var tinf: dynamic = (cpp_cast(1e9) + 7);

var inf: dynamic = (cpp_cast(1e18) + 7);

var N: dynamic = (4e5 + 5);

var used: dynamic = cpp_array(N);

var tin: dynamic = cpp_array(N);

var tout: dynamic = cpp_array(N);

var up: dynamic = cpp_array(40, N);

var l: dynamic = cpp_uninitialized();

var timer: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_array(N);

func dfs(v: dynamic, p: dynamic = 0) -> dynamic
{
  used[v] = 1;
  up[v][0] = p;
  tin[v] = cpp_update(timer, "++");
  cnt[v] = (((v != p)) * ((cnt[p] + 1)));
  {
    var i: dynamic = 1;
    while ((i <= l))
    {
      up[v][i] = up[up[v][(i - 1)]][(i - 1)];
      i += 1;
    }
  }
  for (var i: dynamic in g[v])
  {
    if ((i != p))
    {
      dfs(i, v);
    }
  }
  tout[v] = cpp_update(timer, "++");
}

func upper(a: dynamic, b: dynamic) -> dynamic
{
  return ((tin[a] <= tin[b]) && (tout[a] >= tout[b]));
}

func lca(a: dynamic, b: dynamic) -> dynamic
{
  if (upper(a, b))
  {
    return a;
  }
  if (upper(b, a))
  {
    return b;
  }
  {
    var i: dynamic = l;
    while ((i >= 0))
    {
      if ((!upper(up[a][i], b)))
      {
        a = up[a][i];
      }
      i -= 1;
    }
  }
  return up[a][0];
}

func solver(second: dynamic, t: dynamic, first: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  var is1: dynamic = (lca(first, second) == first);
  var is2: dynamic = (lca(first, t) == first);
  if ((is1 != is2))
  {
    return 1;
  }
  if (is1)
  {
    ans = max(ans, (cnt[lca(second, t)] - cnt[first]));
  } else if ((lca(first, second) != lca(first, t)))
  {
    ans = max(ans, (cnt[first] - max(cnt[lca(first, second)], cnt[lca(first, t)])));
  } else
  {
    ans = max(ans, ((cnt[first] + cnt[lca(second, t)]) - (2 * cnt[lca(first, t)])));
  }
  return (ans + 1);
}

func solve() -> dynamic
{
  var a: dynamic = cpp_array(3);
  var ans: dynamic = (-inf);
  read(a[0], a[1], a[2]);
  a[0] -= 1;
  a[1] -= 1;
  a[2] -= 1;
  {
    var i: dynamic = 0;
    while ((i < 3))
    {
      {
        var j: dynamic = 0;
        while ((j < 3))
        {
          {
            var z: dynamic = 0;
            while ((z < 3))
            {
              if ((((i != j) && (i != z)) && (j != z)))
              {
                ans = max(ans, solver(a[i], a[j], a[z]));
              }
              z += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
}

func main() -> dynamic
{
  {
    ios.sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
  }
  read(n, q);
  while ((((1 << l)) <= n))
  {
    l += 1;
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var p: dynamic = cpp_uninitialized();
      read(p);
      p -= 1;
      g[i].push_back(p);
      g[p].push_back(i);
      i += 1;
    }
  }
  dfs(0);
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      solve();
      i += 1;
    }
  }
}
