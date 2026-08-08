// Translated from solution.cpp.

var MAXN = (cpp_cast(1e5) + 5);

var MAXM = (cpp_cast(1e6) + 5);

var INF = cpp_cast(1e18);

var vec = cpp_array(MAXM);

var e = cpp_array(MAXN);

var adj = cpp_array(MAXN);

var w = cpp_array(MAXN);

var dist = cpp_array(MAXN);

var dist2 = cpp_array(MAXN);

var w2 = cpp_array(MAXN);

var n: dynamic;

var m: dynamic;

var q: dynamic;

func dijkstraSlow()
{
  var Q: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      dist[i] = INF;
      i += 1;
    }
  }
  dist[1] = 0;
  Q.push(make_pair(0, 1));
  while ((!Q.empty()))
  {
    var cd = (-Q.top().first);
    var v = Q.top().second;
    Q.pop();
    if ((cd != dist[v]))
    {
      continue;
    }
    for (var id in adj[v])
    {
      var to = e[id].second;
      var nd = (cd + w[id]);
      if ((nd < dist[to]))
      {
        dist[to] = nd;
        Q.push(make_pair((-nd), to));
      }
    }
  }
}

func dijkstraFast(lim: dynamic)
{
  {
    var i = 1;
    while ((i <= n))
    {
      dist2[i] = INF;
      i += 1;
    }
  }
  dist2[1] = 0;
  vec[0].push_back(1);
  var ptr = 0;
  while (1)
  {
    while (((ptr <= lim) && vec[ptr].empty()))
    {
      ptr += 1;
    }
    if ((ptr > lim))
    {
      break;
    }
    var v = vec[ptr].back();
    vec[ptr].pop_back();
    if ((dist2[v] != ptr))
    {
      continue;
    }
    for (var id in adj[v])
    {
      var to = e[id].second;
      var nd = (ptr + w2[id]);
      if (((nd <= lim) && (nd < dist2[to])))
      {
        dist2[to] = nd;
        vec[nd].push_back(to);
      }
    }
  }
}

func solve()
{
  scanf("%d %d %d", (&n), (&m), (&q));
  {
    var i = 1;
    while ((i <= m))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d %d %d", (&u), (&v), (&w[i]));
      e[i] = make_pair(u, v);
      adj[u].push_back(i);
      i += 1;
    }
  }
  dijkstraSlow();
  while (cpp_update(q, "--"))
  {
    var tp: dynamic;
    scanf("%d", (&tp));
    if ((tp == 1))
    {
      var x: dynamic;
      scanf("%d", (&x));
      if ((dist[x] == INF))
      {
        printf("-1\n");
      } else
      {
        printf("%lld\n", dist[x]);
      }
    } else
    {
      var k: dynamic;
      scanf("%d", (&k));
      {
        var i = 1;
        while ((i <= m))
        {
          var u = e[i].first;
          var v = e[i].second;
          w2[i] = ((w[i] + dist[u]) - dist[v]);
          i += 1;
        }
      }
      {
        var i = 1;
        while ((i <= k))
        {
          var x: dynamic;
          scanf("%d", (&x));
          w[x] += 1;
          w2[x] += 1;
          i += 1;
        }
      }
      dijkstraFast(k);
      {
        var i = 1;
        while ((i <= n))
        {
          if ((dist[i] != INF))
          {
            dist[i] += dist2[i];
          }
          i += 1;
        }
      }
    }
  }
}

func main()
{
  var tt = 1;
  while (cpp_update(tt, "--"))
  {
    solve();
  }
  return 0;
}
