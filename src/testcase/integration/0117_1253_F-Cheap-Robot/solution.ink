// Translated from solution.cpp.

class Edge
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var val: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array((3 * 100055));

func cmp(x1: dynamic, x2: dynamic) -> dynamic
{
  return (x1.val < x2.val);
}

var head: dynamic = cpp_array(100055);

var to: dynamic = cpp_array((6 * 100055));

var nex: dynamic = cpp_array((6 * 100055));

var w: dynamic = cpp_array((6 * 100055));

var edge: dynamic = cpp_uninitialized();

func addEdge(x: dynamic, y: dynamic, z: dynamic) -> dynamic
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

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

func init() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      head[i] = 0;
      i += 1;
    }
  }
  edge = 0;
}

var vis: dynamic = cpp_array(100055);

var d: dynamic = cpp_array(100055);

class node
{
  var x: dynamic = cpp_uninitialized();
  var val: dynamic = cpp_uninitialized();
  func node(x: dynamic, val: dynamic) -> dynamic
  {
      self->x = cpp_construct(x);
      self->val = cpp_construct(val);
    }
}

var que: dynamic = cpp_uninitialized();

var pre: dynamic = cpp_array(100055);

func findd(x: dynamic) -> dynamic
{
  if ((pre[x] == x))
  {
    return x;
  }
  return cpp_assign(pre[x], "=", findd(pre[x]));
}

var f: dynamic = cpp_array(18, 100055);

var g: dynamic = cpp_array(18, 100055);

var dep: dynamic = cpp_array(100055);

var N: dynamic = cpp_uninitialized();

func dfs(u: dynamic, fa: dynamic) -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      g[u][i] = g[g[u][(i - 1)]][(i - 1)];
      f[u][i] = max(f[u][(i - 1)], f[g[u][(i - 1)]][(i - 1)]);
      i += 1;
    }
  }
  {
    var i: dynamic = head[u];
    while (i)
    {
      var v: dynamic = to[i];
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

func lca(x: dynamic, y: dynamic) -> dynamic
{
  if ((dep[x] > dep[y]))
  {
    swap(x, y);
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = N;
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
    var i: dynamic = N;
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

func main() -> dynamic
{
  read(n, m, k, q);
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
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
    var i: dynamic = (k + 1);
    while ((i <= n))
    {
      d[i] = 0x7f7f7f7f7f7f7f7f;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      que.push(node(i, 0));
      i += 1;
    }
  }
  while (que.size())
  {
    var u: dynamic = que.top().x;
    que.pop();
    if (vis[u])
    {
      continue;
    }
    vis[u] = true;
    {
      var i: dynamic = head[u];
      while (i)
      {
        var v: dynamic = to[i];
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
    var i: dynamic = 1;
    while ((i <= m))
    {
      e[i].val += ((d[e[i].x] + d[e[i].y]));
      i += 1;
    }
  }
  sort((e + 1), ((e + 1) + m), cmp);
  init();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      pre[i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      x = e[i].x;
      var xx: dynamic = findd(x);
      y = e[i].y;
      var yy: dynamic = findd(y);
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
