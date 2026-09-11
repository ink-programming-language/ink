// Translated from solution.cpp.

var MAXN: dynamic = (1e5 + 5);

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

class E
{
  var next: dynamic = cpp_uninitialized();
  var to: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array((MAXN << 1));

var ecnt: dynamic = cpp_uninitialized();

var G: dynamic = cpp_array(MAXN);

func addEdge(u: dynamic, v: dynamic) -> dynamic
{
  e[cpp_update(ecnt, "++")] = [G[u], v];
  G[u] = ecnt;
}

func addEdge2(u: dynamic, v: dynamic) -> dynamic
{
  addEdge(u, v);
  addEdge(v, u);
}

var clr: dynamic = cpp_array(MAXN);

func dfs(u: dynamic) -> dynamic
{
  {
    var i: dynamic = G[u];
    while (i)
    {
      var v: dynamic = e[i].to;
      if ((~clr[v]))
      {
        if ((clr[v] == clr[u]))
        {
          return false;
        } else
        {
          i = e[i].next;
          continue;
        }
      }
      clr[v] = (clr[u] ^ 1);
      if ((!dfs(v)))
      {
        return false;
      }
      i = e[i].next;
    }
  }
  return true;
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  scanf("%d%d", (&N), (&M));
  {
    i = 1;
    while ((i <= M))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d%d", (&u), (&v));
      addEdge2(u, v);
      i += 1;
    }
  }
  memset(clr, -1, cpp_sizeof((clr)));
  clr[1] = 0;
  var cnt: dynamic = 0;
  if (dfs(1))
  {
    {
      i = 1;
      while ((i <= N))
      {
        if (clr[i])
        {
          cnt += 1;
        }
        i += 1;
      }
    }
    cnt = (cpp_cast(cnt) * ((N - cnt)));
  } else
  {
    cnt = ((cpp_cast(N) * ((N - 1))) / 2);
  }
  write((cnt - M));
  return 0;
}
