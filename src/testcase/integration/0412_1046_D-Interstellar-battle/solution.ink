// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var x: dynamic;

var y: dynamic;

var a = cpp_array(100005);

class Edge
{
  var v: dynamic;
  var next: dynamic;
}

var h: dynamic;

var pool = cpp_array((100005 << 1));

var tot: dynamic;

func addEdge(u: dynamic, v: dynamic)
{
  var p = (&pool[cpp_update(tot, "++")]);
  p->v = v;
  p->next = h[u];
  h[u] = p;
}

var ans: dynamic;

var um: dynamic;

var r: dynamic;

var fa = cpp_array(100005);

var son = cpp_array(100005);

func dfs(u: dynamic, father: dynamic)
{
  fa[u] = father;
  ans += (((1 - a[u])) * a[fa[u]]);
  {
    var p = h[u];
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

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lf", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
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
    var i = 1;
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
