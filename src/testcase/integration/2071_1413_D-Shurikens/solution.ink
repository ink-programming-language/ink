// Translated from solution.cpp.

var MAXN = (2e5 + 10);

var n: dynamic;

var a = cpp_array(MAXN);

var vis = cpp_array(MAXN);

var nxt = cpp_array(MAXN);

func get_nxt(u: dynamic)
{
  return if ((nxt[u] == u)) u else cpp_assign(nxt[u], "=", get_nxt(nxt[u]));
}

var h: dynamic;

var t: dynamic;

var q = cpp_array(MAXN);

func main()
{
  scanf("%d", (&n));
  var op: dynamic;
  {
    var i = 1;
    while ((i <= (n * 2)))
    {
      read(op);
      if ((op == cpp_char("+")))
      {
        vis[i] = 1;
      } else
      {
        scanf("%d", (&a[i]));
      }
      i += 1;
    }
  }
  vis[((n * 2) + 1)] = 1;
  {
    var i = 0;
    while ((i <= ((n * 2) + 1)))
    {
      nxt[i] = (i + (!vis[i]));
      i += 1;
    }
  }
  q[cpp_assign(h, "=", cpp_assign(t, "=", n))] = pair(0x3f3f3f3f, 0);
  {
    var i = 1;
    var j: dynamic;
    var k: dynamic;
    while ((i <= (n * 2)))
    {
      if ((!vis[i]))
      {
        j = lower_bound((q + h), ((q + t) + 1), pair(a[i], i))->second;
        k = get_nxt(j);
        if ((k > i))
        {
          printf("NO\n");
          return 0;
        } else
        {
          a[k] = a[i];
          nxt[k] = (k + 1);
        }
        while ((a[i] > q[h].first))
        {
          h += 1;
        }
        q[cpp_update(h, "--")] = pair(a[i], i);
      }
      i += 1;
    }
  }
  printf("YES\n");
  {
    var i = 1;
    while ((i <= (n * 2)))
    {
      if (vis[i])
      {
        printf("%d ", a[i]);
      }
      i += 1;
    }
  }
  printf("\n");
  return 0;
}
