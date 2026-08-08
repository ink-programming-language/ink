// Translated from solution.cpp.

var maxn = 3e5;

var n: dynamic;

var m: dynamic;

var f = cpp_array(maxn);

var diameter = cpp_array(maxn);

func find(x: dynamic)
{
  return if ((x == f[x])) x else cpp_assign(f[x], "=", find(f[x]));
}

var G = cpp_array(maxn);

var d1 = cpp_array(maxn);

var d2 = cpp_array(maxn);

var vis = cpp_array(maxn);

var t: dynamic;

var q: dynamic;

func bfs1(u: dynamic, d: dynamic)
{
  t += 1;
  q.push(u);
  d[u] = 0;
  vis[u] = t;
  while ((!q.empty()))
  {
    u = q.front();
    q.pop();
    {
      var i = 0;
      while ((i < G[u].size()))
      {
        var v = G[u][i];
        if ((vis[v] == t))
        {
          i += 1;
          continue;
        }
        q.push(v);
        d[v] = (d[u] + 1);
        vis[v] = t;
        i += 1;
      }
    }
  }
  return u;
}

func bfs2(u: dynamic, fa: dynamic, tag: dynamic)
{
  var ret = -1;
  t += 1;
  q.push(u);
  d2[u] = 0;
  vis[u] = t;
  while ((!q.empty()))
  {
    u = q.front();
    q.pop();
    f[u] = fa;
    if (((d1[u] + d2[u]) == tag))
    {
      if (((d1[u] == (tag / 2)) || (d2[u] == (tag / 2))))
      {
        ret = u;
      }
    }
    {
      var i = 0;
      while ((i < G[u].size()))
      {
        var v = G[u][i];
        if ((vis[v] == t))
        {
          i += 1;
          continue;
        }
        q.push(v);
        d2[v] = (d2[u] + 1);
        vis[v] = t;
        i += 1;
      }
    }
  }
  return ret;
}

func bfs3(u: dynamic)
{
  var fa = u;
  t += 1;
  q.push(u);
  vis[u] = t;
  while ((!q.empty()))
  {
    u = q.front();
    q.pop();
    f[u] = fa;
    {
      var i = 0;
      while ((i < G[u].size()))
      {
        var v = G[u][i];
        if ((vis[v] == t))
        {
          i += 1;
          continue;
        }
        q.push(v);
        vis[v] = t;
        i += 1;
      }
    }
  }
}

func main()
{
  var Q: dynamic;
  scanf("%d%d%d", (&n), (&m), (&Q));
  {
    var i = 0;
    while ((i < m))
    {
      var a: dynamic;
      var b: dynamic;
      scanf("%d%d", (&a), (&b));
      a -= 1;
      b -= 1;
      G[a].push_back(b);
      G[b].push_back(a);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if ((!vis[i]))
      {
        var x = bfs1(i, d1);
        var y = bfs1(x, d1);
        var z = bfs2(y, y, d1[y]);
        diameter[z] = (d1[z] + d2[z]);
        bfs3(z);
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < Q))
    {
      var op: dynamic;
      scanf("%d", (&op));
      if ((op == 1))
      {
        var x: dynamic;
        scanf("%d", (&x));
        x -= 1;
        x = find(x);
        printf("%d\n", diameter[x]);
      } else
      {
        var x: dynamic;
        var y: dynamic;
        scanf("%d%d", (&x), (&y));
        x -= 1;
        y -= 1;
        x = find(x);
        y = find(y);
        if ((x != y))
        {
          if ((diameter[x] > diameter[y]))
          {
            var t1 = (diameter[x] / 2);
            var t2 = (diameter[x] - t1);
            var t3 = (diameter[y] / 2);
            var t4 = (diameter[y] - t3);
            f[y] = x;
            diameter[x] = max(max(diameter[x], diameter[y]), ((t2 + t4) + 1));
          } else
          {
            var t1 = (diameter[x] / 2);
            var t2 = (diameter[x] - t1);
            var t3 = (diameter[y] / 2);
            var t4 = (diameter[y] - t3);
            f[x] = y;
            diameter[y] = max(max(diameter[x], diameter[y]), ((t2 + t4) + 1));
          }
        }
      }
      i += 1;
    }
  }
}
