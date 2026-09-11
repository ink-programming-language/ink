// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(100005);

class Edge
{
  var v: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
}

var h: dynamic = cpp_uninitialized();

var pool: dynamic = cpp_array((100005 << 1));

var tot: dynamic = cpp_uninitialized();

func addEdge(u: dynamic, v: dynamic) -> dynamic
{
  var p: dynamic = (&pool[cpp_update(tot, "++")]);
  p->v = v;
  p->next = h[u];
  h[u] = p;
}

var ans: dynamic = cpp_uninitialized();

var um: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var fa: dynamic = cpp_array(100005);

var son: dynamic = cpp_array(100005);

func dfs(u: dynamic, father: dynamic) -> dynamic
{
  fa[u] = father;
  ans += (((1 - a[u])) * a[fa[u]]);
  {
    var p: dynamic = h[u];
    while (p)
    {
      if ((p->v != father))
      {
        dfs(p->v, u);
        son[u] += ((1 - a[p->v]));
      }
      p = p->next;
    }
  }
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lf", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= (n - 1)))
    {
      scanf("%d%d", (&x), (&y));
      x += 1;
      y += 1;
      addEdge(x, y);
      addEdge(y, x);
      i += 1;
    }
  }
  a[0] = 1;
  scanf("%d", (&m));
  dfs(1, 0);
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      scanf("%d%lf", (&um), (&r));
      um += 1;
      ans += (a[fa[um]] * ((-((r - a[um])))));
      ans += (son[um] * ((r - a[um])));
      son[fa[um]] -= ((r - a[um]));
      a[um] = r;
      printf("%.5lf\n", ans);
      i += 1;
    }
  }
  return 0;
}
