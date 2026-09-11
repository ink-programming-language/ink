// Translated from solution.cpp.

var maxn: dynamic = cpp_expression("#inc");

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var head: dynamic = cpp_array(maxn);

var top: dynamic = cpp_uninitialized();

class E
{
  var to: dynamic = cpp_uninitialized();
  var nxt: dynamic = cpp_uninitialized();
}

var edge: dynamic = cpp_array((maxn << 1));

func insert(u: dynamic, v: dynamic) -> dynamic
{
  edge[cpp_update(top, "++")] = [v, head[u]];
  head[u] = top;
}

var cur: dynamic = cpp_uninitialized();

func dfs(u: dynamic, pre: dynamic, d: dynamic) -> dynamic
{
  if ((d > (k / 2)))
  {
    cur += 1;
  }
  {
    var i: dynamic = head[u];
    while (i)
    {
      var v: dynamic = edge[i].to;
      if ((v == pre))
      {
        i = edge[i].nxt;
        continue;
      }
      dfs(v, u, (d + 1));
      i = edge[i].nxt;
    }
  }
}

func main() -> dynamic
{
  read(n, k);
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_cast(2e9);
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      read(u, v);
      insert(u, v);
      insert(v, u);
      i += 1;
    }
  }
  if ((k & 1))
  {
    {
      var u: dynamic = 1;
      while ((u <= n))
      {
        {
          var i: dynamic = head[u];
          while (i)
          {
            cur = 0;
            var v: dynamic = edge[i].to;
            dfs(u, v, 0);
            dfs(v, u, 0);
            ans = min(ans, cur);
            i = edge[i].nxt;
          }
        }
        u += 1;
      }
    }
  } else
  {
    {
      var u: dynamic = 1;
      while ((u <= n))
      {
        cur = 0;
        dfs(u, 0, 0);
        ans = min(cur, ans);
        u += 1;
      }
    }
  }
  write(ans);
  return 0;
}
