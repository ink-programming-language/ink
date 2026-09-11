// Translated from solution.cpp.

var out: dynamic = cpp_array(100010);

var deg: dynamic = cpp_array(100010);

var q: dynamic = cpp_array(100010);

var vis: dynamic = cpp_array((100010 * 3));

class Node
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
}

var ans: dynamic = cpp_array((3 * 100010));

var cnt: dynamic = cpp_uninitialized();

func dfs(u: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var now: dynamic = cpp_uninitialized();
  while ((!q[u].empty()))
  {
    now = q[u].front();
    q[u].pop();
    v = now.first;
    if (vis[now.second])
    {
      continue;
    }
    vis[now.second] = 1;
    dfs(v);
    ans[cpp_update(cnt, "++")].a = u;
    ans[cnt].b = v;
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  var num: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      scanf("%d%d", (&a), (&b));
      q[a].push(make_pair(b, cpp_update(num, "++")));
      q[b].push(make_pair(a, num));
      deg[a] += 1;
      deg[b] += 1;
      i += 1;
    }
  }
  var last: dynamic = -1;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((deg[i] & 1))
      {
        if ((last == -1))
        {
          last = i;
        } else
        {
          q[last].push(make_pair(i, cpp_update(num, "++")));
          q[i].push(make_pair(last, num));
          last = -1;
          deg[last] += 1;
          deg[i] += 1;
        }
      }
      i += 1;
    }
  }
  if ((num & 1))
  {
    q[1].push(make_pair(1, cpp_update(num, "++")));
  }
  dfs(1);
  write(cnt, "\n");
  {
    var i: dynamic = 1;
    while ((i <= cnt))
    {
      if ((i & 1))
      {
        printf("%d %d\n", ans[i].a, ans[i].b);
      } else
      {
        printf("%d %d\n", ans[i].b, ans[i].a);
      }
      i += 1;
    }
  }
  return 0;
}
