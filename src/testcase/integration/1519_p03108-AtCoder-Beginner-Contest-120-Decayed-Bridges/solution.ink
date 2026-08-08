// Translated from solution.cpp.

var MAXN = (1e5 + 5);

var n: dynamic;

var m: dynamic;

var fa = cpp_array(MAXN);

class EDGE
{
  var u: dynamic;
  var v: dynamic;
}

var e = cpp_array(MAXN);

var ans = cpp_array(MAXN);

var sz = cpp_array(MAXN);

func find(x: dynamic)
{
  if ((x == fa[x]))
  {
    return x;
  }
  return cpp_assign(fa[x], "=", find(fa[x]));
}

func main()
{
  read(n, m);
  {
    var i = 1;
    while ((i <= m))
    {
      read(e[i].u, e[i].v);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      fa[i] = i;
      sz[i] = 1;
      i += 1;
    }
  }
  ans[m] = ((cpp_cast(n) * ((n - 1))) / 2);
  {
    var i = m;
    while ((i >= 1))
    {
      var u = find(e[i].u);
      var v = find(e[i].v);
      if ((u == v))
      {
        ans[(i - 1)] = ans[i];
      } else
      {
        ans[(i - 1)] = (ans[i] - (sz[v] * sz[u]));
        sz[v] += sz[u];
        fa[u] = v;
      }
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      write(ans[i], "\n");
      i += 1;
    }
  }
  return 0;
}
