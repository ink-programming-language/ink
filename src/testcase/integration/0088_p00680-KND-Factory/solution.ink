// Translated from solution.cpp.

func dump()
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; #de");
}

func repi(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=int(a);i<int(b);i++)");
}

func peri(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=int(b);i-->int(a);)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <b");
}

func per(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <b");
}

func all(c: dynamic)
{
  return cpp_expression("#include <bits/");
}

var mp = cpp_expression("#include");

var mt = cpp_expression("#include <");

func operator_shift_left(os: dynamic, p: dynamic)
{
  return (((((os << cpp_char("(")) << p.first) << cpp_char(",")) << p.second) << cpp_char(")"));
}

func print_tuple(argument_0: dynamic, argument_1: dynamic)
{
}

func print_tuple(os: dynamic, t: dynamic)
{
  print_tuple(os, t);
  ((os << (if (cpp_sizeof(Cdr)) "," else "")) << get(t));
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  print_tuple((os << cpp_char("(")), t);
  return (os << cpp_char(")"));
}

func operator_shift_left(os: dynamic, c: dynamic)
{
  (os << cpp_char("["));
  {
    var i = begin(c);
    while ((i != end(c)))
    {
      ((os << (if ((i == begin(c))) "" else " ")) << (*i));
      i += 1;
    }
  }
  return (os << cpp_char("]"));
}

var INF = 1e9;

var MOD = (1e9 + 7);

var EPS = 1e-9;

func GaussJordan(a: dynamic, b: dynamic, x: dynamic)
{
  var n = a.size();
  var a = cpp_construct(n, vd((n + 1)));
  rep(i, n)[i] = a[i][n];
  return true;
}

class Edge
{
  var src: dynamic;
  var dst: dynamic;
  var cost: dynamic;
  var cap: dynamic;
  var flow: dynamic;
  func Edge()
  {
    }
  func Edge(s: dynamic, d: dynamic, co: dynamic, ca: dynamic = 0, f: dynamic = 0)
  {
      this->src = cpp_construct(s);
      this->dst = cpp_construct(d);
      this->cost = cpp_construct(co);
      this->cap = cpp_construct(ca);
      this->flow = cpp_construct(f);
    }
}

func operator_less(a: dynamic, b: dynamic)
{
  return (a.cost < b.cost);
}

func operator_greater(a: dynamic, b: dynamic)
{
  return (a.cost > b.cost);
}

class Graph
{
  var es: dynamic;
  var head: dynamic;
  var next: dynamic;
  func Graph()
  {
    }
  func Graph(n: dynamic)
  {
      this->head = cpp_construct(n, -1);
    }
  func AddEdge(u: dynamic, v: dynamic, co: dynamic, ca: dynamic)
  {
      es.emplace_back(u, v, co, ca);
      next.push_back(head[u]);
      head[u] = (es.size() - 1);
      es.emplace_back(v, u, (-co), 0);
      next.push_back(head[v]);
      head[v] = (es.size() - 1);
    }
}

func MinCostFlow(g: dynamic, tap: dynamic, sink: dynamic, flow: dynamic)
{
  var n = g.head.size();
  var res = 0;
  while ((flow > EPS))
  {
    var prev = cpp_construct(n, -1);
    var pq: dynamic;
    pq.emplace(-1, tap, 0);
    while (pq.size())
    {
      var cur = pq.top();
      pq.pop();
      if ((cur.cost > (cost[cur.dst] - EPS)))
      {
        continue;
      }
      cost[cur.dst] = cur.cost;
      prev[cur.dst] = cur.src;
      {
        var i = g.head[cur.dst];
        while ((i != -1))
        {
          var e = g.es[i];
          if (((e.cap - e.flow) == 0))
          {
            i = g.next[i];
            continue;
          }
          pq.emplace(i, e.dst, (((cost[e.src] + e.cost) + pots[e.src]) - pots[e.dst]));
          i = g.next[i];
        }
      }
    }
    if ((cost[sink] == INF))
    {
      return -1;
    }
    rep(i, n)[i] += cost[i];
    var augment = flow;
    {
      var v = sink;
      while ((v != tap))
      {
        var e = g.es[prev[v]];
        augment = min(augment, (e.cap - e.flow));
        v = g.es[prev[v]].src;
      }
    }
    if ((augment < EPS))
    {
      return -1;
    }
    {
      var v = sink;
      while ((v != tap))
      {
        var i = prev[v];
        g.es[i].flow += augment;
        g.es[(i ^ 1)].flow -= augment;
        v = g.es[prev[v]].src;
      }
    }
    flow -= augment;
    res += (augment * pots[sink]);
  }
  return res;
}

func solve()
{
  var n: dynamic;
  var s: dynamic;
  var t: dynamic;
  var f: dynamic;
  read(n, s, t, f);
  {
    GaussJordan(a, b, cs);
  }
  var res = MinCostFlow(g, s, t, f);
  if ((res == -1))
  {
    puts("impossible");
  } else
  {
    printf("%.10f\n", res);
  }
}

func main()
{
  var tc: dynamic;
  read(tc);
  rep(cpp_name, tc);
  solve();
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    copy(all(a[i]), begin(a[i]));
    a[i][n] = b[i];
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var p = i;
    repi(j, (i + 1), n);
    if ((abs(a[p][i]) < abs(a[j][i])))
    {
      p = j;
    }
    if ((abs(a[p][i]) < EPS))
    {
      return false;
    }
    swap(a[i], a[p]);
    peri(j, i, (n + 1))[i][j] /= a[i][i];
    rep(j, n);
    if ((j != i))
    {
      peri(k, i, (n + 1))[j][k] -= (a[j][i] * a[i][k]);
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      cpp_statement("rep(j,n)");
      read(a[i][j]);
      read(b[i]);
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var m: dynamic;
    read(m);
    rep(j, m);
    read(ds[j]);
    rep(j, m);
    read(fs[j]);
    rep(j, m).AddEdge(i, ds[j], abs((cs[ds[j]] - cs[i])), fs[j]);
  }
