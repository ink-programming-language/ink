// Translated from solution.cpp.

var maxn = (1e5 + 10);

class bkn
{
  var to: dynamic;
  var next: dynamic;
}

var e = cpp_array((maxn * 2));

var n: dynamic;

var m: dynamic;

var c = cpp_array(maxn);

var head = cpp_array(maxn);

var tot: dynamic;

var vis = cpp_array(2, maxn);

var in_cpp = cpp_array(maxn);

var ans = cpp_array(maxn);

var cnt: dynamic;

var h: dynamic;

var win: dynamic;

func add(a: dynamic, b: dynamic)
{
  e[cpp_update(tot, "++")].to = b;
  e[tot].next = head[a];
  head[a] = tot;
}

func dfs(x: dynamic, now: dynamic)
{
  if (((!c[x]) && (now == 1)))
  {
    win = 1;
    ans[cpp_update(cnt, "++")] = x;
    return;
  }
  in_cpp[x] = 1;
  {
    var i = head[x];
    while (i)
    {
      var y = e[i].to;
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

func main()
{
  scanf("%d%d", (&n), (&m));
  var ok = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&c[i]));
      if ((!c[i]))
      {
        ok = 1;
      }
      {
        var j = 1;
        while ((j <= c[i]))
        {
          var x: dynamic;
          scanf("%d", (&x));
          add(i, x);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var s: dynamic;
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
      var i = cnt;
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
