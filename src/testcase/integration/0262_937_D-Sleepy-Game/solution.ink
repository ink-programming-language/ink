// Translated from solution.cpp.

var maxn: dynamic = (1e5 + 10);

class bkn
{
  var to: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array((maxn * 2));

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var c: dynamic = cpp_array(maxn);

var head: dynamic = cpp_array(maxn);

var tot: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(2, maxn);

var in_cpp: dynamic = cpp_array(maxn);

var ans: dynamic = cpp_array(maxn);

var cnt: dynamic = cpp_uninitialized();

var h: dynamic = cpp_uninitialized();

var win: dynamic = cpp_uninitialized();

func add(a: dynamic, b: dynamic) -> dynamic
{
  e[cpp_update(tot, "++")].to = b;
  e[tot].next = head[a];
  head[a] = tot;
}

func dfs(x: dynamic, now: dynamic) -> dynamic
{
  if (((!c[x]) && (now == 1)))
  {
    win = 1;
    ans[cpp_update(cnt, "++")] = x;
    return;
  }
  in_cpp[x] = 1;
  {
    var i: dynamic = head[x];
    while (i)
    {
      var y: dynamic = e[i].to;
      if (in_cpp[y])
      {
        h = 1;
      }
      if (vis[y][(now ^ 1)])
      {
        i = e[i].next;
        continue;
      }
      vis[y][(now ^ 1)] = 1;
      dfs(y, (now ^ 1));
      if (win)
      {
        ans[cpp_update(cnt, "++")] = x;
        return;
      }
      i = e[i].next;
    }
  }
  in_cpp[x] = 0;
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  var ok: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&c[i]));
      if ((!c[i]))
      {
        ok = 1;
      }
      {
        var j: dynamic = 1;
        while ((j <= c[i]))
        {
          var x: dynamic = cpp_uninitialized();
          scanf("%d", (&x));
          add(i, x);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var s: dynamic = cpp_uninitialized();
  scanf("%d", (&s));
  if ((!ok))
  {
    printf("Draw\n");
    return 0;
  }
  dfs(s, 0);
  if (win)
  {
    printf("Win\n");
    {
      var i: dynamic = cnt;
      while ((i >= 1))
      {
        printf("%d ", ans[i]);
        i -= 1;
      }
    }
    printf("\n");
  } else if (h)
  {
    printf("Draw\n");
  } else
  {
    printf("Lose\n");
  }
}
