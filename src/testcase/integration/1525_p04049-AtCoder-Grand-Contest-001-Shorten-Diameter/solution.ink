// Translated from solution.cpp.

var maxn = cpp_expression("#inc");

var n: dynamic;

var k: dynamic;

var head = cpp_array(maxn);

var top: dynamic;

class E
{
  var to: dynamic;
  var nxt: dynamic;
}

var edge = cpp_array((maxn << 1));

func insert(u: dynamic, v: dynamic)
{
  edge[cpp_update(top, "++")] = [v, head[u]];
  head[u] = top;
}

var cur: dynamic;

func dfs(u: dynamic, pre: dynamic, d: dynamic)
{
  if ((d > (k / 2)))
  {
    cur += 1;
  }
  {
    var i = head[u];
    while (i)
    {
      var v = edge[i].to;
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

func main()
{
  read(n, k);
  var u: dynamic;
  var v: dynamic;
  var ans = cpp_cast(2e9);
  {
    var i = 1;
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
      var u = 1;
      while ((u <= n))
      {
        {
          var i = head[u];
          while (i)
          {
            cur = 0;
            var v = edge[i].to;
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
      var u = 1;
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
