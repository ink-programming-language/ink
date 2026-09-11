// Translated from solution.cpp.

var MAXN: dynamic = (cpp_cast(1e5) + 5);

var MAXM: dynamic = (cpp_cast(1e6) + 5);

var INF: dynamic = cpp_cast(1e18);

var vec: dynamic = cpp_array(MAXM);

var e: dynamic = cpp_array(MAXN);

var adj: dynamic = cpp_array(MAXN);

var w: dynamic = cpp_array(MAXN);

var dist: dynamic = cpp_array(MAXN);

var dist2: dynamic = cpp_array(MAXN);

var w2: dynamic = cpp_array(MAXN);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

func dijkstraSlow() -> dynamic
{
  var Q: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
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
    var cd: dynamic = (-Q.top().first);
    var v: dynamic = Q.top().second;
    Q.pop();
    if ((cd != dist[v]))
    {
      continue;
    }
    for (var id: dynamic in adj[v])
    {
      var to: dynamic = e[id].second;
      var nd: dynamic = (cd + w[id]);
      if ((nd < dist[to]))
      {
        dist[to] = nd;
        Q.push(make_pair((-nd), to));
      }
    }
  }
}

func dijkstraFast(lim: dynamic) -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      dist2[i] = INF;
      i += 1;
    }
  }
  dist2[1] = 0;
  vec[0].push_back(1);
  var cpp_ptr: dynamic = 0;
  while (1)
  {
    while (((cpp_ptr <= lim) && vec[cpp_ptr].empty()))
    {
      cpp_ptr += 1;
    }
    if ((cpp_ptr > lim))
    {
      break;
    }
    var v: dynamic = vec[cpp_ptr].back();
    vec[cpp_ptr].pop_back();
    if ((dist2[v] != cpp_ptr))
    {
      continue;
    }
    for (var id: dynamic in adj[v])
    {
      var to: dynamic = e[id].second;
      var nd: dynamic = (cpp_ptr + w2[id]);
      if (((nd <= lim) && (nd < dist2[to])))
      {
        dist2[to] = nd;
        vec[nd].push_back(to);
      }
    }
  }
}

func solve() -> dynamic
{
  scanf("%d %d %d", (&n), (&m), (&q));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d %d %d", (&u), (&v), (&w[i]));
      e[i] = make_pair(u, v);
      adj[u].push_back(i);
      i += 1;
    }
  }
  dijkstraSlow();
  while (cpp_update(q, "--"))
  {
    var tp: dynamic = cpp_uninitialized();
    scanf("%d", (&tp));
    if ((tp == 1))
    {
      var x: dynamic = cpp_uninitialized();
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
      var k: dynamic = cpp_uninitialized();
      scanf("%d", (&k));
      {
        var i: dynamic = 1;
        while ((i <= m))
        {
          var u: dynamic = e[i].first;
          var v: dynamic = e[i].second;
          w2[i] = ((w[i] + dist[u]) - dist[v]);
          i += 1;
        }
      }
      {
        var i: dynamic = 1;
        while ((i <= k))
        {
          var x: dynamic = cpp_uninitialized();
          scanf("%d", (&x));
          w[x] += 1;
          w2[x] += 1;
          i += 1;
        }
      }
      dijkstraFast(k);
      {
        var i: dynamic = 1;
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

func main() -> dynamic
{
  var tt: dynamic = 1;
  while (cpp_update(tt, "--"))
  {
    solve();
  }
  return 0;
}
