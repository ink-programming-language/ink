// Translated from solution.cpp.

var maxn = (1000 + 10);

var maxm = (3000 + 10);

class Edge
{
  var v: dynamic;
  var l: dynamic;
  var r: dynamic;
  var next: dynamic;
  func Edge(v: dynamic = 0, l: dynamic = 0, r: dynamic = 0, next: dynamic = 0)
  {
      this->v = cpp_construct(v);
      this->l = cpp_construct(l);
      this->r = cpp_construct(r);
      this->next = cpp_construct(next);
    }
}

var edges = cpp_array((maxm << 1));

var head = cpp_array(maxn);

var nEdge: dynamic;

var n: dynamic;

var m: dynamic;

var a = cpp_array(maxm);

var b = cpp_array(maxm);

var vis = cpp_array(maxn);

var cnt: dynamic;

func AddEdges(u: dynamic, v: dynamic, l: dynamic, r: dynamic)
{
  edges[cpp_update(nEdge, "++")] = Edge(v, l, r, head[u]);
  head[u] = nEdge;
  edges[cpp_update(nEdge, "++")] = Edge(u, l, r, head[v]);
  head[v] = nEdge;
}

func dfs(u: dynamic, L: dynamic, R: dynamic)
{
  if ((u == n))
  {
    return true;
  }
  vis[u] = cnt;
  {
    var k = head[u];
    while ((k != -1))
    {
      var v = edges[k].v;
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

func solve()
{
  var ans = 0;
  cnt = 0;
  memset(vis, 0, cpp_sizeof((vis)));
  sort(a, (a + m));
  sort(b, (b + m));
  var L: dynamic;
  var R: dynamic;
  var mid: dynamic;
  {
    var i = 0;
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

func main()
{
  scanf("%d%d", (&n), (&m));
  memset(head, 0xff, cpp_sizeof((head)));
  nEdge = -1;
  var u: dynamic;
  var v: dynamic;
  var l: dynamic;
  var r: dynamic;
  {
    var i = 0;
    while ((i < m))
    {
      scanf("%d%d%d%d", (&u), (&v), (&l), (&r));
      AddEdges(u, v, l, r);
      a[i] = l;
      b[i] = r;
      i += 1;
    }
  }
  var ans = solve();
  if ((ans == 0))
  {
    printf("Nice work, Dima!\n");
  } else
  {
    printf("%d\n", ans);
  }
  return 0;
}
