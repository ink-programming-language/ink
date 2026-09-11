// Translated from solution.cpp.

func db(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> using");
}

func db2(x: dynamic, y: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; #define d");
}

func db3(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; #define db(x) cerr << #x << \"=\" <<");
}

func dbv(v: dynamic) -> dynamic
{
  cpp_macro("cerr << #v << \"=\"; for (auto _x : v) cerr << _x << \", \"; cerr << endl");
}

func dba(a: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("cerr << #a << \"=\"; for (int _i = 0; _i < (n); ++_i) cerr << a[_i] << \", \"; cerr << endl");
}

func operator_shift_left(os: dynamic, x: dynamic) -> dynamic
{
  return (((((os << "(") << x.first) << ",") << x.second) << ")");
}

class Shop
{
  var s: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
}

var mp: dynamic = cpp_array(35, 35);

class Mcmf
{
  var INFC: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var src: dynamic = cpp_uninitialized();
  var sink: dynamic = cpp_uninitialized();
  var pi: dynamic = cpp_uninitialized();
  var dist: dynamic = cpp_uninitialized();
  var fromEdge: dynamic = cpp_uninitialized();
  var edges: dynamic = cpp_uninitialized();
  var G: dynamic = cpp_uninitialized();
  func Mcmf(n: dynamic, src: dynamic = -1, sink: dynamic = -1) -> dynamic
  {
      self->n = cpp_construct(n);
      self->src = cpp_construct(src);
      self->sink = cpp_construct(sink);
      G.resize(n);
      dist.resize(n);
      pi.resize(n);
      fromEdge.resize(n);
    }
  func addEdge(from_cpp: dynamic, to: dynamic, cap: dynamic, cost: dynamic) -> dynamic
  {
      G[from_cpp].push_back(edges.size());
      edges.push_back([to, 0, cap, cost]);
      G[to].push_back(edges.size());
      edges.push_back([from_cpp, 0, 0, (-cost)]);
    }
  func cheapestPath() -> dynamic
  {
      var pq: dynamic = cpp_uninitialized();
      fill(dist.begin(), (dist.begin() + n), INFC);
      dist[src] = 0;
      pq.emplace(dist[src], src);
      while ((!pq.empty()))
      {
        var u: dynamic = pq.top().second;
        var cdist: dynamic = pq.top().first;
        pq.pop();
        if ((cdist > dist[u]))
        {
          continue;
        }
        for (var x: dynamic in G[u])
        {
          var e: dynamic = edges[x];
          if ((e.f == e.cap))
          {
            continue;
          }
          var v: dynamic = e.to;
          var val: dynamic = (((dist[u] + pi[u]) - pi[v]) + e.cost);
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
      var cost: dynamic = (dist[sink] + pi[sink]);
      {
        var k: dynamic = 0;
        while ((k < n))
        {
          pi[k] = min(INFC, (pi[k] + dist[k]));
          k += 1;
        }
      }
      return cost;
    }
  func maxFlow() -> dynamic
  {
      var ret: dynamic = cpp_construct(1, 0);
      var totflow: dynamic = 0;
      var LIM: dynamic = 27905;
      while ((totflow < LIM))
      {
        var cost: dynamic = cheapestPath();
        if ((cost >= INFC))
        {
          break;
        }
        var flow: dynamic = INT_MAX;
        {
          var x: dynamic = sink;
          while ((x != src))
          {
            var e: dynamic = fromEdge[x];
            flow = min(flow, (edges[e].cap - edges[e].f));
            x = edges[(e ^ 1)].to;
          }
        }
        {
          var x: dynamic = sink;
          while ((x != src))
          {
            var e: dynamic = fromEdge[x];
            edges[e].f += flow;
            edges[(e ^ 1)].f -= flow;
            x = edges[(e ^ 1)].to;
          }
        }
        flow = min(flow, (LIM - totflow));
        totflow += flow;
        {
          var i: dynamic = 0;
          while ((i < flow))
          {
            ret.push_back(cost);
            i += 1;
          }
        }
      }
      return ret;
    }
  func initPi() -> dynamic
  {
      fill(pi.begin(), (pi.begin() + n), 0);
    }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  var vars: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var k: dynamic = cpp_uninitialized();
      scanf("%d", (&k));
      shops[i].resize(k);
      {
        var j: dynamic = 0;
        while ((j < k))
        {
          var s: dynamic = shops[i][j];
          scanf("%d%d%lld", (&s.s), (&s.p), (&s.c));
          mp[i][j] = cpp_update(vars, "++");
          j += 1;
        }
      }
      mp[i][k] = cpp_update(vars, "++");
      sort(shops[i].begin(), shops[i].end(), __cpp_lambda_1);
      for (var s: dynamic in shops[i])
      {
        shopSizes[i].push_back(s.s);
      }
      i += 1;
    }
  }
  var var_cpp: dynamic = __cpp_lambda_2;
  var m: dynamic = cpp_uninitialized();
  scanf("%d", (&m));
  var constraints: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var w: dynamic = cpp_uninitialized();
      scanf("%d%d%d", (&u), (&v), (&w));
      u -= 1;
      v -= 1;
      {
        var j: dynamic = 0;
        while ((j < shops[v].size()))
        {
          var s: dynamic = shops[v][j].s;
          var k: dynamic = (lower_bound(shopSizes[u].begin(), shopSizes[u].end(), (s - w)) - shopSizes[u].begin());
          constraints.emplace_back(make_pair(u, k), make_pair(v, j));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var CAP: dynamic = 1e8;
  var g: dynamic = cpp_construct((vars + 2), vars, (vars + 1));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
  for (var c: dynamic in constraints)
  {
    g.addEdge(var_cpp(c.second.first, c.second.second), var_cpp(c.first.first, c.first.second), CAP, 0);
  }
  var costs: dynamic = g.maxFlow();
  var pcost: dynamic = cpp_construct((costs.size() + 1));
  {
    var i: dynamic = 0;
    while ((i < costs.size()))
    {
      pcost[(i + 1)] = (pcost[i] + costs[i]);
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  scanf("%d", (&q));
  while (cpp_update(q, "--"))
  {
    var qa: dynamic = cpp_uninitialized();
    scanf("%lld", (&qa));
    var i: dynamic = (lower_bound(costs.begin(), costs.end(), qa) - costs.begin());
    if ((i == costs.size()))
    {
      printf("-1\n");
    } else
    {
      var ans: dynamic = ((qa * ((i - 1))) - pcost[i]);
      assert((ans >= 0));
      printf("%lld\n", ans);
    }
  }
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  return (a.s < b.s);
}

func __cpp_lambda_2(a: dynamic, b: dynamic) -> dynamic
{
  return mp[a][b];
}
