// Translated from solution.cpp.

class Edge
{
  var x: dynamic;
  var y: dynamic;
  var val: dynamic;
}

var e = cpp_array((3 * 100055));

func cmp(x1: dynamic, x2: dynamic)
{
  return (x1.val < x2.val);
}

var head = cpp_array(100055);

var to = cpp_array((6 * 100055));

var nex = cpp_array((6 * 100055));

var w = cpp_array((6 * 100055));

var edge: dynamic;

func addEdge(x: dynamic, y: dynamic, z: dynamic)
{
  to[cpp_update(edge, "++")] = y;
  w[edge] = z;
  nex[edge] = head[x];
  head[x] = edge;
  to[cpp_update(edge, "++")] = x;
  w[edge] = z;
  nex[edge] = head[y];
  head[y] = edge;
}

var n: dynamic;

var m: dynamic;

var k: dynamic;

var q: dynamic;

func init()
{
  {
    var i = 1;
    while ((i <= n))
    {
      head[i] = 0;
      i += 1;
    }
  }
  edge = 0;
}

var vis = cpp_array(100055);

var d = cpp_array(100055);

class node
{
  var x: dynamic;
  var val: dynamic;
  func node(x: dynamic, val: dynamic)
  {
      this->x = cpp_construct(x);
      this->val = cpp_construct(val);
    }
}

var que: dynamic;

var pre = cpp_array(100055);

func findd(x: dynamic)
{
  if ((pre[x] == x))
  {
    return x;
  }
  return cpp_assign(pre[x], "=", findd(pre[x]));
}

var f = cpp_array(18, 100055);

var g = cpp_array(18, 100055);

var dep = cpp_array(100055);

var N: dynamic;

func dfs(u: dynamic, fa: dynamic)
{
  {
    var i = 1;
    while ((i <= N))
    {
      g[u][i] = g[g[u][(i - 1)]][(i - 1)];
      f[u][i] = max(f[u][(i - 1)], f[g[u][(i - 1)]][(i - 1)]);
      i += 1;
    }
  }
  {
    var i = head[u];
    while (i)
    {
      var v = to[i];
      if ((v == fa))
      {
        i = nex[i];
        continue;
      }
      dep[v] = (dep[u] + 1);
      g[v][0] = u;
      f[v][0] = w[i];
      dfs(v, u);
      i = nex[i];
    }
  }
}

func lca(x: dynamic, y: dynamic)
{
  if ((dep[x] > dep[y]))
  {
    swap(x, y);
  }
  var ans = 0;
  {
    var i = N;
    while ((i >= 0))
    {
      if ((dep[g[y][i]] >= dep[x]))
      {
        ans = max(ans, f[y][i]);
        y = g[y][i];
      }
      i -= 1;
    }
  }
  if ((x == y))
  {
    return ans;
  }
  {
    var i = N;
    while ((i >= 0))
    {
      if ((g[x][i] != g[y][i]))
      {
        ans = max(f[x][i], ans);
        ans = max(f[y][i], ans);
        x = g[x][i];
        y = g[y][i];
      }
      i -= 1;
    }
  }
  if ((x != y))
  {
    ans = max(ans, f[x][0]);
    ans = max(ans, f[y][0]);
  }
  return ans;
}

func main()
{
  read(n, m, k, q);
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  {
    var i = 1;
    while ((i <= m))
    {
      scanf("%d%d%d", (&x), (&y), (&z));
      addEdge(x, y, z);
      e[i].x = x;
      e[i].y = y;
      e[i].val = z;
      i += 1;
    }
  }
  {
    var i = (k + 1);
    while ((i <= n))
    {
      d[i] = 0x7f7f7f7f7f7f7f7f;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= k))
    {
      que.push(node(i, 0));
      i += 1;
    }
  }
  while (que.size())
  {
    var u = que.top().x;
    que.pop();
    if (vis[u])
    {
      continue;
    }
    vis[u] = true;
    {
      var i = head[u];
      while (i)
      {
        var v = to[i];
        if ((d[v] > (d[u] + w[i])))
        {
          d[v] = (d[u] + w[i]);
          que.push(node(v, d[v]));
        }
        i = nex[i];
      }
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      e[i].val += ((d[e[i].x] + d[e[i].y]));
      i += 1;
    }
  }
  sort((e + 1), ((e + 1) + m), cmp);
  init();
  {
    var i = 1;
    while ((i <= n))
    {
      pre[i] = i;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      x = e[i].x;
      var xx = findd(x);
      y = e[i].y;
      var yy = findd(y);
      if ((xx != yy))
      {
        pre[xx] = yy;
        addEdge(x, y, e[i].val);
      }
      i += 1;
    }
  }
  N = ceil(log2(n));
  dep[1] = 1;
  dfs(1, 0);
  while (cpp_update(q, "--"))
  {
    scanf("%d%d", (&x), (&y));
    printf("%lld\n", lca(x, y));
  }
  return 0;
}
