// Translated from solution.cpp.

var q: dynamic;

var fa = cpp_array((10000 + 10));

var dfn = cpp_array((10000 + 10));

var n: dynamic;

var m: dynamic;

var tot: dynamic;

var dcnt: dynamic;

var S: dynamic;

var T: dynamic;

var cnt = 1;

var vis = cpp_array(((10000 * 4) + 10));

func Read(x: dynamic)
{
  var c: dynamic;
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

var dist = cpp_array(((10000 * 3) + 10));

var vd = cpp_array(((10000 * 3) + 10));

var vis = cpp_array(((10000 * 3) + 10));

class node
{
  var v: dynamic;
  var cap: dynamic;
  var next: dynamic;
  var back: dynamic;
}

var adj: dynamic;

var edge = cpp_array(((10000 * 20) + 10));

var ecnt = edge;

func addedge(u: dynamic, v: dynamic, cap: dynamic)
{
  var p = cpp_update(ecnt, "++");
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

func spfa(S: dynamic)
{
  var i: dynamic;
  var u: dynamic;
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
      var p = adj[u];
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

func dfs(u: dynamic, augu: dynamic)
{
  if ((u == T))
  {
    return augu;
  }
  var mind = (tot - 1);
  var delta: dynamic;
  var augv = 0;
  {
    var p = adj[u];
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

func sap()
{
  var flow = 0;
  var i: dynamic;
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
  var v: dynamic;
  var c: dynamic;
  var next: dynamic;
}

var adj: dynamic;

var edge = cpp_array(((10000 * 4) + 10));

var ecnt = edge;

var pre: dynamic;

func addedge(u: dynamic, v: dynamic, c: dynamic)
{
  var p = cpp_update(ecnt, "++");
  p->v = v;
  p->c = c;
  p->next = adj[u];
  adj[u] = p;
}

func dfs(u: dynamic)
{
  dfn[u] = cpp_update(dcnt, "++");
  {
    var p = adj[u];
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
          var x = u;
          var num = 0;
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

func read()
{
  Read(n);
  Read(m);
  var i: dynamic;
  var u: dynamic;
  var v: dynamic;
  var c: dynamic;
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

func solve()
{
  dfs(1);
  MAXFLOW.addedge((m + 3), T, 0x7fffffff);
  tot = ((cnt + m) + 2);
  {
    var i = 1;
    while ((i <= m))
    {
      MAXFLOW.addedge(S, i, 1);
      i += 1;
    }
  }
}

func main()
{
  read();
  solve();
  printf("%d\n", MAXFLOW.sap());
}
