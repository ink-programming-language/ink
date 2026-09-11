// Translated from solution.cpp.

var rng: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var N: dynamic = 200200;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ANS: dynamic = cpp_array(N);

var g: dynamic = cpp_array(N);

var G: dynamic = cpp_array(N);

var deg: dynamic = cpp_array(N);

var q: dynamic = cpp_array(N);

var topQ: dynamic = cpp_uninitialized();

var id: dynamic = cpp_array(N);

var a: dynamic = cpp_array(N);

var Q: dynamic = cpp_array(N);

var b: dynamic = cpp_array(N);

var pref: dynamic = cpp_array(N);

func read() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&g[i]));
      g[i] -= 1;
      deg[g[i]] += 1;
      i += 1;
    }
  }
  scanf("%d", (&m));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var x: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%lld%d", (&x), (&v));
      v -= 1;
      Q[v].push_back(make_pair(x, i));
      i += 1;
    }
  }
}

func solveTree(v: dynamic) -> dynamic
{
  var big: dynamic = -1;
  for (var u: dynamic in G[v])
  {
    if (((big == -1) || (cpp_cast(a[id[u]].size()) > cpp_cast(a[id[big]].size()))))
    {
      big = u;
    }
  }
  if ((big == -1))
  {
    id[v] = v;
  } else
  {
    id[v] = id[big];
  }
  var sz: dynamic = cpp_cast(a[id[v]].size());
  for (var u: dynamic in G[v])
  {
    if ((u == big))
    {
      continue;
    }
    var z: dynamic = id[u];
    reverse(a[z].begin(), a[z].end());
    {
      var i: dynamic = 0;
      while ((i < cpp_cast(a[z].size())))
      {
        a[id[v]][((sz - 1) - i)] += a[z][i];
        i += 1;
      }
    }
  }
  a[id[v]].push_back(1);
  for (var t: dynamic in Q[v])
  {
    var x: dynamic = t.first;
    if ((x <= sz))
    {
      ANS[t.second] = a[id[v]][(sz - x)];
    }
  }
  var u: dynamic = g[v];
  G[u].push_back(v);
  deg[u] -= 1;
  if ((deg[u] == 0))
  {
    q[cpp_update(topQ, "++")] = u;
  }
}

func solveCycle(cycle: dynamic) -> dynamic
{
  reverse(cycle.begin(), cycle.end());
  var k: dynamic = cpp_cast(cycle.size());
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      b[i].clear();
      pref[i].clear();
      i += 1;
    }
  }
  {
    var t: dynamic = 0;
    while ((t < k))
    {
      var v: dynamic = cycle[t];
      var big: dynamic = -1;
      for (var u: dynamic in G[v])
      {
        if (((big == -1) || (cpp_cast(a[id[u]].size()) > cpp_cast(a[id[big]].size()))))
        {
          big = u;
        }
      }
      if ((big == -1))
      {
        id[v] = v;
      } else
      {
        id[v] = id[big];
      }
      var sz: dynamic = cpp_cast(a[id[v]].size());
      for (var u: dynamic in G[v])
      {
        if ((u == big))
        {
          continue;
        }
        var z: dynamic = id[u];
        reverse(a[z].begin(), a[z].end());
        {
          var i: dynamic = 0;
          while ((i < cpp_cast(a[z].size())))
          {
            a[id[v]][((sz - 1) - i)] += a[z][i];
            i += 1;
          }
        }
      }
      a[id[v]].push_back(1);
      reverse(a[id[v]].begin(), a[id[v]].end());
      {
        var i: dynamic = 0;
        while ((i <= sz))
        {
          var p: dynamic = (((t + i)) % k);
          b[p].push_back(make_pair(i, a[id[v]][i]));
          i += 1;
        }
      }
      t += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      sort(b[i].begin(), b[i].end());
      pref[i].push_back(0);
      for (var t: dynamic in b[i])
      {
        pref[i].push_back((pref[i].back() + t.second));
      }
      i += 1;
    }
  }
  {
    var t: dynamic = 0;
    while ((t < k))
    {
      var v: dynamic = cycle[t];
      for (var z: dynamic in Q[v])
      {
        var x: dynamic = z.first;
        var xx: dynamic = cpp_uninitialized();
        if ((x > cpp_cast(1e7)))
        {
          xx = (x - (((((x - cpp_cast(1e7))) / k)) * k));
        } else
        {
          xx = x;
        }
        var p: dynamic = (((xx + t)) % k);
        var pos: dynamic = (lower_bound(b[p].begin(), b[p].end(), make_pair(xx, N)) - b[p].begin());
        ANS[z.second] = pref[p][pos];
      }
      t += 1;
    }
  }
}

func main() -> dynamic
{
  read();
  {
    var v: dynamic = 0;
    while ((v < n))
    {
      if ((deg[v] == 0))
      {
        q[cpp_update(topQ, "++")] = v;
      }
      v += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < topQ))
    {
      var v: dynamic = q[i];
      solveTree(v);
      i += 1;
    }
  }
  {
    var v: dynamic = 0;
    while ((v < n))
    {
      if ((deg[v] == 0))
      {
        v += 1;
        continue;
      }
      var all: dynamic = cpp_uninitialized();
      var u: dynamic = v;
      while (true)
      {
        all.push_back(u);
        u = g[u];
        if (!(((u != v))))
        {
          break;
        }
      }
      solveCycle(all);
      for (var u: dynamic in all)
      {
        deg[u] = 0;
      }
      v += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      printf("%d\n", ANS[i]);
      i += 1;
    }
  }
  return 0;
}
