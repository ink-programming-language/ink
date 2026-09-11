// Translated from solution.cpp.

var maxn: dynamic = 3e5;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(maxn);

var diameter: dynamic = cpp_array(maxn);

func find(x: dynamic) -> dynamic
{
  return  ((x == f[x])) ? x : cpp_assign(f[x], "=", find(f[x]));
}

var G: dynamic = cpp_array(maxn);

var d1: dynamic = cpp_array(maxn);

var d2: dynamic = cpp_array(maxn);

var vis: dynamic = cpp_array(maxn);

var t: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

func bfs1(u: dynamic, d: dynamic) -> dynamic
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
      var i: dynamic = 0;
      while ((i < G[u].size()))
      {
        var v: dynamic = G[u][i];
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

func bfs2(u: dynamic, fa: dynamic, tag: dynamic) -> dynamic
{
  var ret: dynamic = -1;
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
      var i: dynamic = 0;
      while ((i < G[u].size()))
      {
        var v: dynamic = G[u][i];
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

func bfs3(u: dynamic) -> dynamic
{
  var fa: dynamic = u;
  t += 1;
  q.push(u);
  vis[u] = t;
  while ((!q.empty()))
  {
    u = q.front();
    q.pop();
    f[u] = fa;
    {
      var i: dynamic = 0;
      while ((i < G[u].size()))
      {
        var v: dynamic = G[u][i];
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

func main() -> dynamic
{
  var Q: dynamic = cpp_uninitialized();
  scanf("%d%d%d", (&n), (&m), (&Q));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      scanf("%d%d", (&a), (&b));
      a -= 1;
      b -= 1;
      G[a].push_back(b);
      G[b].push_back(a);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((!vis[i]))
      {
        var x: dynamic = bfs1(i, d1);
        var y: dynamic = bfs1(x, d1);
        var z: dynamic = bfs2(y, y, d1[y]);
        diameter[z] = (d1[z] + d2[z]);
        bfs3(z);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < Q))
    {
      var op: dynamic = cpp_uninitialized();
      scanf("%d", (&op));
      if ((op == 1))
      {
        var x: dynamic = cpp_uninitialized();
        scanf("%d", (&x));
        x -= 1;
        x = find(x);
        printf("%d\n", diameter[x]);
      } else
      {
        var x: dynamic = cpp_uninitialized();
        var y: dynamic = cpp_uninitialized();
        scanf("%d%d", (&x), (&y));
        x -= 1;
        y -= 1;
        x = find(x);
        y = find(y);
        if ((x != y))
        {
          if ((diameter[x] > diameter[y]))
          {
            var t1: dynamic = (diameter[x] / 2);
            var t2: dynamic = (diameter[x] - t1);
            var t3: dynamic = (diameter[y] / 2);
            var t4: dynamic = (diameter[y] - t3);
            f[y] = x;
            diameter[x] = max(max(diameter[x], diameter[y]), ((t2 + t4) + 1));
          } else
          {
            var t1: dynamic = (diameter[x] / 2);
            var t2: dynamic = (diameter[x] - t1);
            var t3: dynamic = (diameter[y] / 2);
            var t4: dynamic = (diameter[y] - t3);
            f[x] = y;
            diameter[y] = max(max(diameter[x], diameter[y]), ((t2 + t4) + 1));
          }
        }
      }
      i += 1;
    }
  }
}
