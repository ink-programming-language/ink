// Translated from solution.cpp.

func REP(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for(int i=s;i<n;i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include<b");
}

var EPS = cpp_expression("#includ");

func equals(a: dynamic, b: dynamic)
{
  return cpp_expression("#include<bits/stdc+");
}

func LT(a: dynamic, b: dynamic)
{
  return ((!equals(a, b)) && (a < b));
}

func LTE(a: dynamic, b: dynamic)
{
  return (equals(a, b) || (a < b));
}

class Point
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  func operator_subtract(p: dynamic)
  {
      return [(x - p.x), (y - p.y), (z - p.z)];
    }
  func operator_less(p: dynamic)
  {
      return (if (((!equals(z, p.z)))) (z > p.z) else (if (((!equals(y, p.y)))) (y < p.y) else LT(x, p.x)));
    }
}

class Edge
{
  var to: dynamic;
  var cap: dynamic;
  var cost: dynamic;
  var rev: dynamic;
  func Edge(to: dynamic = 0, cap: dynamic = 0, cost: dynamic = 0, rev: dynamic = 0)
  {
      this->to = cpp_construct(to);
      this->cap = cpp_construct(cap);
      this->cost = cpp_construct(cost);
      this->rev = cpp_construct(rev);
    }
}

var MAX_V = 1100;

var IINF = INT_MAX;

var V: dynamic;

var G = cpp_array(MAX_V);

var h = cpp_array(MAX_V);

var dist = cpp_array(MAX_V);

var prevv = cpp_array(MAX_V);

var preve = cpp_array(MAX_V);

func add_edge(from_cpp: dynamic, to: dynamic, cap: dynamic, cost: dynamic)
{
  G[from_cpp].push_back(Edge(to, cap, cost, G[to].size()));
  G[to].push_back(Edge(from_cpp, 0, (-cost), (G[from_cpp].size() - 1)));
}

func min_cost_flow(s: dynamic, t: dynamic)
{
  var res = 0;
  fill(h, (h + V), 0);
  while (1)
  {
    var Q: dynamic;
    fill(dist, (dist + V), IINF);
    dist[s] = 0;
    Q.push(ii(0, s));
    while ((!Q.empty()))
    {
      var p = Q.top();
      Q.pop();
      var v = p.second;
      if ((dist[v] < p.first))
      {
        continue;
      }
      {
        var i = 0;
        while ((i < G[v].size()))
        {
          var e = G[v][i];
          if (((LT(0, e.cap) && (!equals((((dist[v] + e.cost) + h[v]) - h[e.to]), dist[e.to]))) && (dist[e.to] > (((dist[v] + e.cost) + h[v]) - h[e.to]))))
          {
            dist[e.to] = (((dist[v] + e.cost) + h[v]) - h[e.to]);
            prevv[e.to] = v;
            preve[e.to] = i;
            Q.push(ii(dist[e.to], e.to));
          }
          i += 1;
        }
      }
    }
    rep(v, V)[v] += dist[v];
    if ((h[t] >= 0))
    {
      break;
    }
    var d = IINF;
    {
      var v = t;
      while ((v != s))
      {
        d = min(d, G[prevv[v]][preve[v]].cap);
        v = prevv[v];
      }
    }
    res += (d * h[t]);
    {
      var v = t;
      while ((v != s))
      {
        var e = G[prevv[v]][preve[v]];
        e.cap -= d;
        G[v][e.rev].cap += d;
        v = prevv[v];
      }
    }
  }
  return res;
}

var N: dynamic;

var K: dynamic;

var ps = cpp_array(100);

var MAX = 10000;

func getDist(p: dynamic)
{
  return sqrt((((p.x * p.x) + (p.y * p.y)) + (p.z * p.z)));
}

func compute()
{
  if ((N <= K))
  {
    puts("0");
    return;
  }
  var counter: dynamic;
  var failed = false;
  rep(i, N)[cpp_cast(ps[i].z)] += 1;
  {
    var it = counter.begin();
    while ((it != counter.end()))
    {
      if ((it->second > K))
      {
        failed = true;
        break;
      }
      it += 1;
    }
  }
  if (failed)
  {
    puts("-1");
    return;
  }
  rep(i, (N * 4))[i].clear();
  var source = (N * 2);
  var sink = (source + 2);
  V = (sink + 1);
  add_edge(source, (source + 1), K, 0);
  rep(i, N);
  rep(j, N);
  if (((i != j) && (ps[i].z > ps[j].z)))
  {
    add_edge((N + i), j, 1, getDist((ps[i] - ps[j])));
  }
  printf("%.10f\n", (min_cost_flow(source, sink) + (N * MAX)));
}

func main()
{
  while (cpp_comma(((cin >> N) >> K), (N | K)))
  {
    cpp_statement("rep(i,N)");
    read(ps[i].x, ps[i].y, ps[i].z);
    compute();
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    add_edge((source + 1), i, 1, 0);
    add_edge(i, (N + i), 1, (-MAX));
    add_edge((N + i), sink, 1, 0);
  }
