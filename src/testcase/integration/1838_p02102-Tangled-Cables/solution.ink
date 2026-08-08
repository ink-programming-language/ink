// Translated from solution.cpp.

var EPS = (1e-10);

var INF = 252521;

func equals(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <iostream>");
}

var par: dynamic;

var rnk: dynamic;

func init(n: dynamic)
{
  par.resize(n);
  rnk.resize(n);
  {
    var i = 0;
    while ((i < n))
    {
      par[i] = i;
      rnk[i] = 0;
      i += 1;
    }
  }
}

func find(x: dynamic)
{
  if ((par[x] == x))
  {
    return x;
  }
  return cpp_assign(par[x], "=", find(par[x]));
}

func unite(x: dynamic, y: dynamic)
{
  x = find(x);
  y = find(y);
  if ((x == y))
  {
    return;
  }
  if ((rnk[x] < rnk[y]))
  {
    par[x] = y;
  } else
  {
    par[y] = x;
    if ((rnk[x] == rnk[y]))
    {
      rnk[x] += 1;
    }
  }
}

func same(x: dynamic, y: dynamic)
{
  return ((find(x) == find(y)));
}

class Edge
{
  var u: dynamic;
  var v: dynamic;
  var w: dynamic;
  func Edge()
  {
    }
  func Edge(u: dynamic, v: dynamic, w: dynamic)
  {
      this->u = cpp_construct();
      this->v = cpp_construct();
      this->w = cpp_construct();
    }
}

class Data
{
  var u: dynamic;
  var v: dynamic;
  var c: dynamic;
  var t: dynamic;
  func Data()
  {
    }
  func Data(u: dynamic, v: dynamic, c: dynamic, t: dynamic)
  {
      this->u = cpp_construct();
      this->v = cpp_construct();
      this->c = cpp_construct();
      this->t = cpp_construct();
    }
}

var N: dynamic;

var M: dynamic;

var d: dynamic;

var es: dynamic;

func kruskal()
{
  sort(es.begin(), es.end(), __cpp_lambda_1);
  init(N);
  var tw = 0;
  for (var e in es)
  {
    if ((!same(e.u, e.v)))
    {
      unite(e.u, e.v);
    } else
    {
      tw += e.w;
    }
  }
  return tw;
}

func c(x: dynamic)
{
  {
    var i = 0;
    while ((i < M))
    {
      es[i].u = d[i].u;
      es[i].v = d[i].v;
      es[i].w = (d[i].t - (x * d[i].c));
      i += 1;
    }
  }
  return (kruskal() >= 0);
}

func solve()
{
  var lb = 0;
  var ub = INF;
  {
    var i = 0;
    while ((i < 50))
    {
      var mid = (((lb + ub)) / 2);
      if (c(mid))
      {
        lb = mid;
      } else
      {
        ub = mid;
      }
      i += 1;
    }
  }
  return ub;
}

func main()
{
  scanf("%d %d", (&N), (&M));
  d.resize(M);
  es.resize(M);
  {
    var i = 0;
    while ((i < M))
    {
      scanf("%d %d %lf %lf", (&d[i].u), (&d[i].v), (&d[i].c), (&d[i].t));
      i += 1;
    }
  }
  var res = solve();
  printf("%.15f\n", (if (equals(res, INF)) 0 else res));
  return 0;
}

func __cpp_lambda_1(e1: dynamic, e2: dynamic)
{
  return (e1.w > e2.w);
}
