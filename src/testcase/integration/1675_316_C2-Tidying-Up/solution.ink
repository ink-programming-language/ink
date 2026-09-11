// Translated from solution.cpp.

var debug: dynamic = cpp_uninitialized();

var inf: dynamic = (1e9 + 5);

var nax: dynamic = 6405;

class Edge
{
  var w: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var rev: dynamic = cpp_uninitialized();
  func Edge(w: dynamic, c: dynamic, v: dynamic, rev: dynamic) -> dynamic
  {
      self->w = cpp_construct(w);
      self->c = cpp_construct(c);
      self->v = cpp_construct(v);
      self->rev = cpp_construct(rev);
    }
}

var odl: dynamic = cpp_array(nax);

var pot: dynamic = cpp_array(nax);

var pop: dynamic = cpp_array(nax);

var pop_kraw: dynamic = cpp_array(nax);

var q: dynamic = cpp_array((nax * 100));

var qbeg: dynamic = cpp_uninitialized();

var qend: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(nax);

var bylo: dynamic = cpp_array(nax);

var kolej: dynamic = cpp_uninitialized();

func init(n: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      v[i].clear();
      i += 1;
    }
  }
}

func AddEdge(a: dynamic, b: dynamic, cap: dynamic, cost: dynamic) -> dynamic
{
  v[a].push_back(Edge(b, cap, cost, (int_cpp(v[b].size()) + ((a == b)))));
  v[b].push_back(Edge(a, 0, (-cost), int_cpp((v[a].size() - 1))));
}

func MinCostMaxFlow(s: dynamic, t: dynamic, n: dynamic) -> dynamic
{
  var flow: dynamic = 0;
  var cost: dynamic = 0;
  while (true)
  {
    {
      var i: dynamic = 0;
      while ((i <= n))
      {
        odl[i] = inf;
        bylo[i] = false;
        i += 1;
      }
    }
    bylo[s] = true;
    odl[s] = 0;
    qbeg = cpp_assign(qend, "=", 0);
    q[cpp_update(qend, "++")] = s;
    while ((qbeg < qend))
    {
      var x: dynamic = q[cpp_update(qbeg, "++")];
      bylo[x] = false;
      var dl: dynamic = v[x].size();
      {
        var i: dynamic = 0;
        while ((i <= ((dl) - 1)))
        {
          if (((v[x][i].c > 0) && (odl[v[x][i].w] > (((odl[x] + pot[x]) - pot[v[x][i].w]) + v[x][i].v))))
          {
            odl[v[x][i].w] = (((odl[x] + pot[x]) - pot[v[x][i].w]) + v[x][i].v);
            if ((!bylo[v[x][i].w]))
            {
              q[cpp_update(qend, "++")] = v[x][i].w;
              bylo[v[x][i].w] = true;
            }
            pop[v[x][i].w] = x;
            pop_kraw[v[x][i].w] = i;
          }
          i += 1;
        }
      }
    }
    if ((odl[t] == inf))
    {
      break;
    }
    var x: dynamic = t;
    var cap: dynamic = inf;
    while ((x != s))
    {
      cap = min(cap, v[pop[x]][pop_kraw[x]].c);
      x = pop[x];
    }
    flow += cap;
    x = t;
    while ((x != s))
    {
      cost += (v[pop[x]][pop_kraw[x]].v * cap);
      v[pop[x]][pop_kraw[x]].c -= cap;
      v[x][v[pop[x]][pop_kraw[x]].rev].c += cap;
      x = pop[x];
    }
  }
  return make_pair(flow, cost);
}

func __cpp_top_level_1() -> dynamic
{
}

var naxn: dynamic = 87;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var t: dynamic = cpp_array(naxn, naxn);

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  debug = (argc > 1);
  scanf("%d%d", (&n), (&m));
  init(((n * m) + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          scanf("%d", (&t[i][j]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          var val: dynamic = ((((i - 1)) * m) + j);
          if ((((i + j)) & 1))
          {
            MinCost.AddEdge(0, val, 1, 0);
            if ((i > 1))
            {
              AddEdge(val, (val - m), 1, (t[i][j] != t[(i - 1)][j]));
            }
            if ((j > 1))
            {
              AddEdge(val, (val - 1), 1, (t[i][j] != t[i][(j - 1)]));
            }
            if ((i < n))
            {
              AddEdge(val, (val + m), 1, (t[i][j] != t[(i + 1)][j]));
            }
            if ((j < m))
            {
              AddEdge(val, (val + 1), 1, (t[i][j] != t[i][(j + 1)]));
            }
          } else
          {
            MinCost.AddEdge(val, ((n * m) + 1), 1, 0);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", MinCost.MinCostMaxFlow(0, ((n * m) + 1), ((n * m) + 1)).second);
  return 0;
}
