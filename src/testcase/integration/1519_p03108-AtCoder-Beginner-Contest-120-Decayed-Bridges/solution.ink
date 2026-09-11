// Translated from solution.cpp.

var MAXN: dynamic = (1e5 + 5);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var fa: dynamic = cpp_array(MAXN);

class EDGE
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array(MAXN);

var ans: dynamic = cpp_array(MAXN);

var sz: dynamic = cpp_array(MAXN);

func find(x: dynamic) -> dynamic
{
  if ((x == fa[x]))
  {
    return x;
  }
  return cpp_assign(fa[x], "=", find(fa[x]));
}

func main() -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      read(e[i].u, e[i].v);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      fa[i] = i;
      sz[i] = 1;
      i += 1;
    }
  }
  ans[m] = ((cpp_cast(n) * ((n - 1))) / 2);
  {
    var i: dynamic = m;
    while ((i >= 1))
    {
      var u: dynamic = find(e[i].u);
      var v: dynamic = find(e[i].v);
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
    var i: dynamic = 1;
    while ((i <= m))
    {
      write(ans[i], "\n");
      i += 1;
    }
  }
  return 0;
}
