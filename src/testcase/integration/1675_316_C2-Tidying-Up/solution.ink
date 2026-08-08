// Translated from solution.cpp.

var debug: dynamic;

var inf = (1e9 + 5);

var nax = 6405;

class Edge
{
  var w: dynamic;
  var c: dynamic;
  var v: dynamic;
  var rev: dynamic;
  func Edge(w: dynamic, c: dynamic, v: dynamic, rev: dynamic)
  {
      this->w = cpp_construct(w);
      this->c = cpp_construct(c);
      this->v = cpp_construct(v);
      this->rev = cpp_construct(rev);
    }
}

var odl = cpp_array(nax);

var pot = cpp_array(nax);

var pop = cpp_array(nax);

var pop_kraw = cpp_array(nax);

var q = cpp_array((nax * 100));

var qbeg: dynamic;

var qend: dynamic;

var v = cpp_array(nax);

var bylo = cpp_array(nax);

var kolej: dynamic;

func init(n: dynamic)
{
  {
    var i = 0;
    while ((i <= n))
    {
      v[i].clear();
      i += 1;
    }
  }
}

func AddEdge(a: dynamic, b: dynamic, cap: dynamic, cost: dynamic)
{
  v[a].push_back(Edge(b, cap, cost, (int_cpp(v[b].size()) + ((a == b)))));
  v[b].push_back(Edge(a, 0, (-cost), int_cpp((v[a].size() - 1))));
}

func MinCostMaxFlow(s: dynamic, t: dynamic, n: dynamic)
{
  var flow = 0;
  var cost = 0;
  while (true)
  {
    {
      var i = 0;
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
      var x = q[cpp_update(qbeg, "++")];
      bylo[x] = false;
      var dl = v[x].size();
      {
        var i = 0;
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
    var x = t;
    var cap = inf;
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

func __cpp_top_level_1()
{
}

var naxn = 87;

var n: dynamic;

var m: dynamic;

var t = cpp_array(naxn, naxn);

func main(argc: dynamic, argv: dynamic)
{
  debug = (argc > 1);
  scanf("%d%d", (&n), (&m));
  init(((n * m) + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
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
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= m))
        {
          var val = ((((i - 1)) * m) + j);
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
