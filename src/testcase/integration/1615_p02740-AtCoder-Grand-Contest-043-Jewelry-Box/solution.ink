// Translated from solution.cpp.

func db(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using");
}

func db2(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; #define d");
}

func db3(x: dynamic, y: dynamic, z: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; #define db(x) cerr << #x << \"=\" <<");
}

func dbv(v: dynamic)
{
  cpp_macro("cerr << #v << \"=\"; for (auto _x : v) cerr << _x << \", \"; cerr << endl");
}

func dba(a: dynamic, n: dynamic)
{
  cpp_macro("cerr << #a << \"=\"; for (int _i = 0; _i < (n); ++_i) cerr << a[_i] << \", \"; cerr << endl");
}

func operator_shift_left(os: dynamic, x: dynamic)
{
  return (((((os << "(") << x.first) << ",") << x.second) << ")");
}

class Shop
{
  var s: dynamic;
  var p: dynamic;
  var c: dynamic;
}

var mp = cpp_array(35, 35);

class Mcmf
{
  var INFC: dynamic;
  var n: dynamic;
  var src: dynamic;
  var sink: dynamic;
  var pi: dynamic;
  var dist: dynamic;
  var fromEdge: dynamic;
  var edges: dynamic;
  var G: dynamic;
  func Mcmf(n: dynamic, src: dynamic = -1, sink: dynamic = -1)
  {
      this->n = cpp_construct(n);
      this->src = cpp_construct(src);
      this->sink = cpp_construct(sink);
      G.resize(n);
      dist.resize(n);
      pi.resize(n);
      fromEdge.resize(n);
    }
  func addEdge(from_cpp: dynamic, to: dynamic, cap: dynamic, cost: dynamic)
  {
      G[from_cpp].push_back(edges.size());
      edges.push_back([to, 0, cap, cost]);
      G[to].push_back(edges.size());
      edges.push_back([from_cpp, 0, 0, (-cost)]);
    }
  func cheapestPath()
  {
      var pq: dynamic;
      fill(dist.begin(), (dist.begin() + n), INFC);
      dist[src] = 0;
      pq.emplace(dist[src], src);
      while ((!pq.empty()))
      {
        var u = pq.top().second;
        var cdist = pq.top().first;
        pq.pop();
        if ((cdist > dist[u]))
        {
          continue;
        }
        for (var x in G[u])
        {
          var e = edges[x];
          if ((e.f == e.cap))
          {
            continue;
          }
          var v = e.to;
          var val = (((dist[u] + pi[u]) - pi[v]) + e.cost);
          if ((val < dist[v]))
          {
            dist[v] = val;
            fromEdge[v] = x;
            pq.emplace(dist[v], v);
          }
        }
      }
      if ((dist[sink] >= INFC))
      {
        return INFC;
      }
      var cost = (dist[sink] + pi[sink]);
      {
        var k = 0;
        while ((k < n))
        {
          pi[k] = min(INFC, (pi[k] + dist[k]));
          k += 1;
        }
      }
      return cost;
    }
  func maxFlow()
  {
      var ret = cpp_construct(1, 0);
      var totflow = 0;
      var LIM = 27905;
      while ((totflow < LIM))
      {
        var cost = cheapestPath();
        if ((cost >= INFC))
        {
          break;
        }
        var flow = INT_MAX;
        {
          var x = sink;
          while ((x != src))
          {
            var e = fromEdge[x];
            flow = min(flow, (edges[e].cap - edges[e].f));
            x = edges[(e ^ 1)].to;
          }
        }
        {
          var x = sink;
          while ((x != src))
          {
            var e = fromEdge[x];
            edges[e].f += flow;
            edges[(e ^ 1)].f -= flow;
            x = edges[(e ^ 1)].to;
          }
        }
        flow = min(flow, (LIM - totflow));
        totflow += flow;
        {
          var i = 0;
          while ((i < flow))
          {
            ret.push_back(cost);
            i += 1;
          }
        }
      }
      return ret;
    }
  func initPi()
  {
      fill(pi.begin(), (pi.begin() + n), 0);
    }
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  var vars = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var k: dynamic;
      scanf("%d", (&k));
      shops[i].resize(k);
      {
        var j = 0;
        while ((j < k))
        {
          var s = shops[i][j];
          scanf("%d%d%lld", (&s.s), (&s.p), (&s.c));
          mp[i][j] = cpp_update(vars, "++");
          j += 1;
        }
      }
      mp[i][k] = cpp_update(vars, "++");
      sort(shops[i].begin(), shops[i].end(), __cpp_lambda_1);
      for (var s in shops[i])
      {
        shopSizes[i].push_back(s.s);
      }
      i += 1;
    }
  }
  var var_cpp = __cpp_lambda_2;
  var m: dynamic;
  scanf("%d", (&m));
  var constraints: dynamic;
  {
    var i = 0;
    while ((i < m))
    {
      var u: dynamic;
      var v: dynamic;
      var w: dynamic;
      scanf("%d%d%d", (&u), (&v), (&w));
      u -= 1;
      v -= 1;
      {
        var j = 0;
        while ((j < shops[v].size()))
        {
          var s = shops[v][j].s;
          var k = (lower_bound(shopSizes[u].begin(), shopSizes[u].end(), (s - w)) - shopSizes[u].begin());
          constraints.emplace_back(make_pair(u, k), make_pair(v, j));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var CAP = 1e8;
  var g = cpp_construct((vars + 2), vars, (vars + 1));
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < shops[i].size()))
        {
          g.addEdge(var_cpp(i, (j + 1)), var_cpp(i, j), CAP, 0);
          g.addEdge(var_cpp(i, j), var_cpp(i, (j + 1)), CAP, shops[i][j].c);
          g.addEdge(var_cpp(i, j), var_cpp(i, (j + 1)), shops[i][j].p, 0);
          j += 1;
        }
      }
      g.addEdge(g.src, var_cpp(i, 0), CAP, 0);
      g.addEdge(var_cpp(i, shops[i].size()), g.sink, CAP, 0);
      i += 1;
    }
  }
  for (var c in constraints)
  {
    g.addEdge(var_cpp(c.second.first, c.second.second), var_cpp(c.first.first, c.first.second), CAP, 0);
  }
  var costs = g.maxFlow();
  var pcost = cpp_construct((costs.size() + 1));
  {
    var i = 0;
    while ((i < costs.size()))
    {
      pcost[(i + 1)] = (pcost[i] + costs[i]);
      i += 1;
    }
  }
  var q: dynamic;
  scanf("%d", (&q));
  while (cpp_update(q, "--"))
  {
    var qa: dynamic;
    scanf("%lld", (&qa));
    var i = (lower_bound(costs.begin(), costs.end(), qa) - costs.begin());
    if ((i == costs.size()))
    {
      printf("-1\n");
    } else
    {
      var ans = ((qa * ((i - 1))) - pcost[i]);
      assert((ans >= 0));
      printf("%lld\n", ans);
    }
  }
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return (a.s < b.s);
}

func __cpp_lambda_2(a: dynamic, b: dynamic)
{
  return mp[a][b];
}
