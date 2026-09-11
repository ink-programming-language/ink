// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var c: dynamic = getchar();
  {
    while ((!isdigit(c)))
    {
      if ((c == cpp_char("-")))
      {
        f = -1;
      }
      c = getchar();
    }
  }
  {
    while (isdigit(c))
    {
      x = ((((x << 1)) + ((x << 3))) + ((c ^ 48)));
      c = getchar();
    }
  }
  return (x * f);
}

class Edge
{
  var to: dynamic = cpp_uninitialized();
  var nxt: dynamic = cpp_uninitialized();
  var type_cpp: dynamic = cpp_uninitialized();
  func Edge() -> dynamic
  {
    }
  func Edge(to: dynamic, nxt: dynamic, type_cpp: dynamic) -> dynamic
  {
      self->to = cpp_construct(to);
      self->nxt = cpp_construct(nxt);
      self->type_cpp = cpp_construct(type_cpp);
    }
}

var edge: dynamic = cpp_array((100099 * 2));

var first: dynamic = cpp_array(100099);

var nume: dynamic = cpp_uninitialized();

func Addedge(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  edge[nume] = Edge(b, first[a], c);
  first[a] = cpp_update(nume, "++");
}

var fa: dynamic = cpp_array(100099);

var son: dynamic = cpp_array(100099);

var size: dynamic = cpp_array(100099);

var up: dynamic = cpp_array(100099);

var down: dynamic = cpp_array(100099);

var top: dynamic = cpp_array(100099);

var n: dynamic = cpp_uninitialized();

var deep: dynamic = cpp_array(100099);

var Pow: dynamic = cpp_array(1000009);

func dfs1(u: dynamic, f: dynamic) -> dynamic
{
  size[u] = 1;
  son[u] = 0;
  {
    var e: dynamic = first[u];
    while ((~e))
    {
      var v: dynamic = edge[e].to;
      if ((v != f))
      {
        deep[v] = (deep[u] + 1);
        dfs1(v, u);
        size[u] += size[v];
        fa[v] = u;
        if ((size[son[u]] < size[v]))
        {
          son[u] = v;
        }
      }
      e = edge[e].nxt;
    }
  }
}

func dfs2(u: dynamic, chain: dynamic) -> dynamic
{
  top[u] = chain;
  if (son[u])
  {
    dfs2(son[u], chain);
  }
  {
    var e: dynamic = first[u];
    while ((~e))
    {
      var v: dynamic = edge[e].to;
      if (((v != son[u]) && (v != fa[u])))
      {
        dfs2(v, v);
      }
      e = edge[e].nxt;
    }
  }
}

func lca(x: dynamic, y: dynamic) -> dynamic
{
  while ((top[x] != top[y]))
  {
    if ((deep[top[x]] > deep[top[y]]))
    {
      x = fa[top[x]];
    } else
    {
      y = fa[top[y]];
    }
  }
  return  ((deep[x] > deep[y])) ? y : x;
}

var ans: dynamic = cpp_uninitialized();

func dfs(u: dynamic) -> dynamic
{
  {
    var e: dynamic = first[u];
    while ((~e))
    {
      var v: dynamic = edge[e].to;
      if ((v == fa[u]))
      {
        e = edge[e].nxt;
        continue;
      }
      dfs(v);
      if ((edge[e].type_cpp == 1))
      {
        ans = (((((ans + Pow[up[v]]) - 1) + 1000000007)) % 1000000007);
      }
      if ((edge[e].type_cpp == 2))
      {
        ans = (((((ans + Pow[down[v]]) - 1) + 1000000007)) % 1000000007);
      }
      up[u] += up[v];
      down[u] += down[v];
      e = edge[e].nxt;
    }
  }
}

func main() -> dynamic
{
  n = read();
  Pow[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= 1000000))
    {
      Pow[i] = (((1 * Pow[(i - 1)]) * 2) % 1000000007);
      i += 1;
    }
  }
  memset(first, -1, cpp_sizeof((first)));
  nume = 0;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var a: dynamic = read();
      var b: dynamic = read();
      var c: dynamic = read();
      if ((!c))
      {
        Addedge(a, b, 0);
        Addedge(b, a, 0);
      } else
      {
        Addedge(a, b, 1);
        Addedge(b, a, 2);
      }
      i += 1;
    }
  }
  size[0] = 0;
  deep[1] = 0;
  dfs1(1, 0);
  dfs2(1, 1);
  memset(up, 0, cpp_sizeof((up)));
  memset(down, 0, cpp_sizeof((down)));
  var K: dynamic = read();
  var last: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i <= K))
    {
      var now: dynamic = read();
      var tmp: dynamic = lca(last, now);
      up[last] += 1;
      up[tmp] -= 1;
      down[now] += 1;
      down[tmp] -= 1;
      last = now;
      i += 1;
    }
  }
  ans = 0;
  dfs(1);
  printf("%I64d\n", ans);
  return 0;
}
