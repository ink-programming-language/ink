// Translated from solution.cpp.

var EPS: dynamic = (1e-10);

var INF: dynamic = 252521;

func equals(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream>");
}

var par: dynamic = cpp_uninitialized();

var rnk: dynamic = cpp_uninitialized();

func init(n: dynamic) -> dynamic
{
  par.resize(n);
  rnk.resize(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      par[i] = i;
      rnk[i] = 0;
      i += 1;
    }
  }
}

func find(x: dynamic) -> dynamic
{
  if ((par[x] == x))
  {
    return x;
  }
  return cpp_assign(par[x], "=", find(par[x]));
}

func unite(x: dynamic, y: dynamic) -> dynamic
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

func same(x: dynamic, y: dynamic) -> dynamic
{
  return ((find(x) == find(y)));
}

class Edge
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  func Edge() -> dynamic
  {
    }
  func Edge(u: dynamic, v: dynamic, w: dynamic) -> dynamic
  {
      self->u = cpp_construct();
      self->v = cpp_construct();
      self->w = cpp_construct();
    }
}

class Data
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  func Data() -> dynamic
  {
    }
  func Data(u: dynamic, v: dynamic, c: dynamic, t: dynamic) -> dynamic
  {
      self->u = cpp_construct();
      self->v = cpp_construct();
      self->c = cpp_construct();
      self->t = cpp_construct();
    }
}

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var es: dynamic = cpp_uninitialized();

func kruskal() -> dynamic
{
  sort(es.begin(), es.end(), __cpp_lambda_1);
  init(N);
  var tw: dynamic = 0;
  for (var e: dynamic in es)
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

func c(x: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
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

func solve() -> dynamic
{
  var lb: dynamic = 0;
  var ub: dynamic = INF;
  {
    var i: dynamic = 0;
    while ((i < 50))
    {
      var mid: dynamic = (((lb + ub)) / 2);
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

func main() -> dynamic
{
  scanf("%d %d", (&N), (&M));
  d.resize(M);
  es.resize(M);
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      scanf("%d %d %lf %lf", (&d[i].u), (&d[i].v), (&d[i].c), (&d[i].t));
      i += 1;
    }
  }
  var res: dynamic = solve();
  printf("%.15f\n", ( (equals(res, INF)) ? 0 : res));
  return 0;
}

func __cpp_lambda_1(e1: dynamic, e2: dynamic) -> dynamic
{
  return (e1.w > e2.w);
}
