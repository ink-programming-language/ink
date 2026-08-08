// Translated from solution.cpp.

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var N = 200200;

var n: dynamic;

var m: dynamic;

var ANS = cpp_array(N);

var g = cpp_array(N);

var G = cpp_array(N);

var deg = cpp_array(N);

var q = cpp_array(N);

var topQ: dynamic;

var id = cpp_array(N);

var a = cpp_array(N);

var Q = cpp_array(N);

var b = cpp_array(N);

var pref = cpp_array(N);

func read()
{
  scanf("%d", (&n));
  {
    var i = 0;
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
    var i = 0;
    while ((i < m))
    {
      var x: dynamic;
      var v: dynamic;
      scanf("%lld%d", (&x), (&v));
      v -= 1;
      Q[v].push_back(make_pair(x, i));
      i += 1;
    }
  }
}

func solveTree(v: dynamic)
{
  var big = -1;
  for (var u in G[v])
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
  var sz = cpp_cast(a[id[v]].size());
  for (var u in G[v])
  {
    if ((u == big))
    {
      continue;
    }
    var z = id[u];
    reverse(a[z].begin(), a[z].end());
    {
      var i = 0;
      while ((i < cpp_cast(a[z].size())))
      {
        a[id[v]][((sz - 1) - i)] += a[z][i];
        i += 1;
      }
    }
  }
  a[id[v]].push_back(1);
  for (var t in Q[v])
  {
    var x = t.first;
    if ((x <= sz))
    {
      ANS[t.second] = a[id[v]][(sz - x)];
    }
  }
  var u = g[v];
  G[u].push_back(v);
  deg[u] -= 1;
  if ((deg[u] == 0))
  {
    q[cpp_update(topQ, "++")] = u;
  }
}

func solveCycle(cycle: dynamic)
{
  reverse(cycle.begin(), cycle.end());
  var k = cpp_cast(cycle.size());
  {
    var i = 0;
    while ((i < k))
    {
      b[i].clear();
      pref[i].clear();
      i += 1;
    }
  }
  {
    var t = 0;
    while ((t < k))
    {
      var v = cycle[t];
      var big = -1;
      for (var u in G[v])
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
      var sz = cpp_cast(a[id[v]].size());
      for (var u in G[v])
      {
        if ((u == big))
        {
          continue;
        }
        var z = id[u];
        reverse(a[z].begin(), a[z].end());
        {
          var i = 0;
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
        var i = 0;
        while ((i <= sz))
        {
          var p = (((t + i)) % k);
          b[p].push_back(make_pair(i, a[id[v]][i]));
          i += 1;
        }
      }
      t += 1;
    }
  }
  {
    var i = 0;
    while ((i < k))
    {
      sort(b[i].begin(), b[i].end());
      pref[i].push_back(0);
      for (var t in b[i])
      {
        pref[i].push_back((pref[i].back() + t.second));
      }
      i += 1;
    }
  }
  {
    var t = 0;
    while ((t < k))
    {
      var v = cycle[t];
      for (var z in Q[v])
      {
        var x = z.first;
        var xx: dynamic;
        if ((x > cpp_cast(1e7)))
        {
          xx = (x - (((((x - cpp_cast(1e7))) / k)) * k));
        } else
        {
          xx = x;
        }
        var p = (((xx + t)) % k);
        var pos = (lower_bound(b[p].begin(), b[p].end(), make_pair(xx, N)) - b[p].begin());
        ANS[z.second] = pref[p][pos];
      }
      t += 1;
    }
  }
}

func main()
{
  read();
  {
    var v = 0;
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
    var i = 0;
    while ((i < topQ))
    {
      var v = q[i];
      solveTree(v);
      i += 1;
    }
  }
  {
    var v = 0;
    while ((v < n))
    {
      if ((deg[v] == 0))
      {
        v += 1;
        continue;
      }
      var all: dynamic;
      var u = v;
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
      for (var u in all)
      {
        deg[u] = 0;
      }
      v += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      printf("%d\n", ANS[i]);
      i += 1;
    }
  }
  return 0;
}
