// Translated from solution.cpp.

var MAXN = (1e5 + 5);

var N: dynamic;

var M: dynamic;

class E
{
  var next: dynamic;
  var to: dynamic;
}

var e = cpp_array((MAXN << 1));

var ecnt: dynamic;

var G = cpp_array(MAXN);

func addEdge(u: dynamic, v: dynamic)
{
  e[cpp_update(ecnt, "++")] = [G[u], v];
  G[u] = ecnt;
}

func addEdge2(u: dynamic, v: dynamic)
{
  addEdge(u, v);
  addEdge(v, u);
}

var clr = cpp_array(MAXN);

func dfs(u: dynamic)
{
  {
    var i = G[u];
    while (i)
    {
      var v = e[i].to;
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

func main()
{
  var i: dynamic;
  scanf("%d%d", (&N), (&M));
  {
    i = 1;
    while ((i <= M))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d%d", (&u), (&v));
      addEdge2(u, v);
      i += 1;
    }
  }
  memset(clr, -1, cpp_sizeof((clr)));
  clr[1] = 0;
  var cnt = 0;
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
