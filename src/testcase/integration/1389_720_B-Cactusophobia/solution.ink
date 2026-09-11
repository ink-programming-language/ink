// Translated from solution.cpp.

var q: dynamic = cpp_uninitialized();

var fa: dynamic = cpp_array((10000 + 10));

var dfn: dynamic = cpp_array((10000 + 10));

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var tot: dynamic = cpp_uninitialized();

var dcnt: dynamic = cpp_uninitialized();

var S: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var cnt: dynamic = 1;

var vis: dynamic = cpp_array(((10000 * 4) + 10));

func Read(x: dynamic) -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  while (cpp_comma(cpp_assign(c, "=", getchar()), (c != EOF)))
  {
    if (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
    {
      x = (c - cpp_char("0"));
      while (cpp_comma(cpp_assign(c, "=", getchar()), ((c >= cpp_char("0")) && (c <= cpp_char("9")))))
      {
        x = (((x * 10) + c) - cpp_char("0"));
      }
      ungetc(c, stdin);
      return;
    }
  }
}

var dist: dynamic = cpp_array(((10000 * 3) + 10));

var vd: dynamic = cpp_array(((10000 * 3) + 10));

var vis: dynamic = cpp_array(((10000 * 3) + 10));

class node
{
  var v: dynamic = cpp_uninitialized();
  var cap: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
  var back: dynamic = cpp_uninitialized();
}

var adj: dynamic = cpp_uninitialized();

var edge: dynamic = cpp_array(((10000 * 20) + 10));

var ecnt: dynamic = edge;

func addedge(u: dynamic, v: dynamic, cap: dynamic) -> dynamic
{
  var p: dynamic = cpp_update(ecnt, "++");
  p->v = v;
  p->cap = cap;
  p->next = adj[u];
  adj[u] = p;
  p = cpp_assign(p->back, "=", cpp_update(ecnt, "++"));
  p->v = u;
  p->cap = 0;
  p->next = adj[v];
  adj[v] = p;
  p->back = (ecnt - 1);
}

func spfa(S: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  {
    i = 1;
    while ((i <= tot))
    {
      dist[i] = 0x7fffffff;
      i += 1;
    }
  }
  dist[S] = 0;
  q.push(S);
  while ((!q.empty()))
  {
    u = q.front();
    q.pop();
    vis[u] = 0;
    {
      var p: dynamic = adj[u];
      while (p)
      {
        if ((p->back->cap && (dist[p->v] > (dist[u] + 1))))
        {
          dist[p->v] = (dist[u] + 1);
          if ((!vis[p->v]))
          {
            q.push(p->v);
            vis[p->v] = 1;
          }
        }
        p = p->next;
      }
    }
  }
}

func dfs(u: dynamic, augu: dynamic) -> dynamic
{
  if ((u == T))
  {
    return augu;
  }
  var mind: dynamic = (tot - 1);
  var delta: dynamic = cpp_uninitialized();
  var augv: dynamic = 0;
  {
    var p: dynamic = adj[u];
    while (p)
    {
      if (p->cap)
      {
        if ((dist[u] == (dist[p->v] + 1)))
        {
          delta = min(p->cap, (augu - augv));
          delta = dfs(p->v, delta);
          p->cap -= delta;
          p->back->cap += delta;
          augv += delta;
          if (((augv == augu) || (dist[S] >= tot)))
          {
            return augv;
          }
        }
        mind = min(dist[p->v], mind);
      }
      p = p->next;
    }
  }
  if ((!augv))
  {
    if ((!cpp_update(vd[dist[u]], "--")))
    {
      dist[S] = tot;
    }
    vd[cpp_assign(dist[u], "=", (mind + 1))] += 1;
  }
  return augv;
}

func sap() -> dynamic
{
  var flow: dynamic = 0;
  var i: dynamic = cpp_uninitialized();
  spfa(T);
  {
    i = 1;
    while ((i <= tot))
    {
      vd[i] = 0;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= tot))
    {
      if ((dist[i] != 0x7fffffff))
      {
        vd[dist[i]] += 1;
      }
      i += 1;
    }
  }
  while ((dist[S] < tot))
  {
    flow += dfs(S, 0x7fffffff);
  }
  return flow;
}

class node
{
  var v: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
}

var adj: dynamic = cpp_uninitialized();

var edge: dynamic = cpp_array(((10000 * 4) + 10));

var ecnt: dynamic = edge;

var pre: dynamic = cpp_uninitialized();

func addedge(u: dynamic, v: dynamic, c: dynamic) -> dynamic
{
  var p: dynamic = cpp_update(ecnt, "++");
  p->v = v;
  p->c = c;
  p->next = adj[u];
  adj[u] = p;
}

func dfs(u: dynamic) -> dynamic
{
  dfn[u] = cpp_update(dcnt, "++");
  {
    var p: dynamic = adj[u];
    while (p)
    {
      if ((p->v != fa[u]))
      {
        if ((!dfn[p->v]))
        {
          pre[p->v] = p;
          fa[p->v] = u;
          dfs(p->v);
          if ((!vis[(p - edge)]))
          {
            MAXFLOW.addedge(p->c, (m + 3), 1);
          }
        } else if ((dfn[p->v] < dfn[u]))
        {
          var x: dynamic = u;
          var num: dynamic = 0;
          cnt += 1;
          MAXFLOW.addedge(p->c, ((cnt + m) + 2), 1);
          while ((x != p->v))
          {
            MAXFLOW.addedge(pre[x]->c, ((cnt + m) + 2), 1);
            vis[(pre[x] - edge)] = 1;
            x = fa[x];
            num += 1;
          }
          MAXFLOW.addedge(((cnt + m) + 2), T, num);
        }
      }
      p = p->next;
    }
  }
}

func read() -> dynamic
{
  Read(n);
  Read(m);
  var i: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  {
    i = 1;
    while ((i <= m))
    {
      Read(u);
      Read(v);
      Read(c);
      addedge(u, v, c);
      addedge(v, u, c);
      i += 1;
    }
  }
  S = (m + 1);
  T = (S + 1);
}

func solve() -> dynamic
{
  dfs(1);
  MAXFLOW.addedge((m + 3), T, 0x7fffffff);
  tot = ((cnt + m) + 2);
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      MAXFLOW.addedge(S, i, 1);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  read();
  solve();
  printf("%d\n", MAXFLOW.sap());
}
