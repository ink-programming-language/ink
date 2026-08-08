// Translated from solution.cpp.

var out = cpp_array(100010);

var deg = cpp_array(100010);

var q = cpp_array(100010);

var vis = cpp_array((100010 * 3));

class Node
{
  var a: dynamic;
  var b: dynamic;
}

var ans = cpp_array((3 * 100010));

var cnt: dynamic;

func dfs(u: dynamic)
{
  var i: dynamic;
  var v: dynamic;
  var now: dynamic;
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

func main()
{
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var a: dynamic;
  var b: dynamic;
  scanf("%d%d", (&n), (&m));
  var num = 0;
  {
    var i = 1;
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
  var last = -1;
  {
    var i = 1;
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
    var i = 1;
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
