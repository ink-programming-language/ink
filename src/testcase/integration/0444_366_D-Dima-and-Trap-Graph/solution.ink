// Translated from solution.cpp.

var maxn: dynamic = (1000 + 10);

var maxm: dynamic = (3000 + 10);

class Edge
{
  var v: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
  func Edge(v: dynamic = 0, l: dynamic = 0, r: dynamic = 0, next: dynamic = 0) -> dynamic
  {
      self->v = cpp_construct(v);
      self->l = cpp_construct(l);
      self->r = cpp_construct(r);
      self->next = cpp_construct(next);
    }
}

var edges: dynamic = cpp_array((maxm << 1));

var head: dynamic = cpp_array(maxn);

var nEdge: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxm);

var b: dynamic = cpp_array(maxm);

var vis: dynamic = cpp_array(maxn);

var cnt: dynamic = cpp_uninitialized();

func AddEdges(u: dynamic, v: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  edges[cpp_update(nEdge, "++")] = Edge(v, l, r, head[u]);
  head[u] = nEdge;
  edges[cpp_update(nEdge, "++")] = Edge(u, l, r, head[v]);
  head[v] = nEdge;
}

func dfs(u: dynamic, L: dynamic, R: dynamic) -> dynamic
{
  if ((u == n))
  {
    return true;
  }
  vis[u] = cnt;
  {
    var k: dynamic = head[u];
    while ((k != -1))
    {
      var v: dynamic = edges[k].v;
      if ((vis[v] == cnt))
      {
        k = edges[k].next;
        continue;
      }
      if (((L < edges[k].l) || (R > edges[k].r)))
      {
        k = edges[k].next;
        continue;
      }
      if (dfs(v, L, R))
      {
        return true;
      }
      k = edges[k].next;
    }
  }
  return false;
}

func solve() -> dynamic
{
  var ans: dynamic = 0;
  cnt = 0;
  memset(vis, 0, cpp_sizeof((vis)));
  sort(a, (a + m));
  sort(b, (b + m));
  var L: dynamic = cpp_uninitialized();
  var R: dynamic = cpp_uninitialized();
  var mid: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      L = a[i];
      R = b[(m - 1)];
      while ((L <= R))
      {
        mid = (((L + R)) >> 1);
        cnt += 1;
        if (dfs(1, a[i], mid))
        {
          ans = max(ans, ((mid - a[i]) + 1));
          L = (mid + 1);
        } else
        {
          R = (mid - 1);
        }
      }
      i += 1;
    }
  }
  return ans;
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  memset(head, 0xff, cpp_sizeof((head)));
  nEdge = -1;
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d%d%d%d", (&u), (&v), (&l), (&r));
      AddEdges(u, v, l, r);
      a[i] = l;
      b[i] = r;
      i += 1;
    }
  }
  var ans: dynamic = solve();
  if ((ans == 0))
  {
    printf("Nice work, Dima!\n");
  } else
  {
    printf("%d\n", ans);
  }
  return 0;
}
