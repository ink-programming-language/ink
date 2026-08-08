// Translated from solution.cpp.

var maxn = (5 * 100005);

var a = cpp_array(maxn);

var n: dynamic;

var m: dynamic;

var k: dynamic;

var lim: dynamic;

var head = cpp_array(maxn);

var Next = cpp_array((maxn * 2));

var to = cpp_array((maxn * 2));

var tot = 0;

var d = cpp_array(maxn);

var fa = cpp_array(maxn);

var size = cpp_array(maxn);

var vis = cpp_array(maxn);

func add(x: dynamic, y: dynamic)
{
  to[cpp_update(tot, "++")] = y;
  Next[tot] = head[x];
  head[x] = tot;
}

func dfs_tree(x: dynamic, f: dynamic)
{
  size[x] = 1;
  {
    var i = head[x];
    while (i)
    {
      var y = to[i];
      if (((y == f) || vis[y]))
      {
        i = Next[i];
        continue;
      }
      d[y] = (d[x] + 1);
      fa[y] = x;
      vis[y] = 1;
      dfs_tree(y, x);
      size[x] += size[y];
      i = Next[i];
    }
  }
}

func main()
{
  scanf("%d%d%d", (&n), (&m), (&k));
  lim = ceil(((cpp_cast(n) * 1.0) / k));
  {
    var i = 1;
    while ((i <= m))
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%d%d", (&x), (&y));
      add(x, y);
      add(y, x);
      i += 1;
    }
  }
  vis[1] = 1;
  d[1] = 1;
  dfs_tree(1, 0);
  var pos = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((d[i] >= lim))
      {
        puts("PATH");
        pos = i;
        printf("%d\n", d[i]);
        while (pos)
        {
          printf("%d ", pos);
          pos = fa[pos];
        }
        printf("\n");
        return 0;
      }
      i += 1;
    }
  }
  var cnt = 0;
  puts("CYCLES");
  {
    var i = 1;
    while ((i <= n))
    {
      if ((size[i] == 1))
      {
        var p1 = 0;
        var p2 = 0;
        {
          var j = head[i];
          while (j)
          {
            var y = to[j];
            if ((y == fa[i]))
            {
              j = Next[j];
              continue;
            }
            if ((!p1))
            {
              p1 = y;
            } else
            {
              if ((!p2))
              {
                p2 = y;
                break;
              }
            }
            j = Next[j];
          }
        }
        if ((d[p1] < d[p2]))
        {
          swap(p1, p2);
        }
        var c1 = ((d[i] - d[p1]) + 1);
        var c2 = ((d[i] - d[p2]) + 1);
        var c3 = ((d[p1] - d[p2]) + 2);
        if (((c1 > 3) && ((c1 % 3) != 0)))
        {
          cnt += 1;
          pos = i;
          while ((pos != fa[p1]))
          {
            a[cnt].push_back(pos);
            pos = fa[pos];
          }
        } else if (((c2 > 3) && ((c2 % 3) != 0)))
        {
          cnt += 1;
          pos = i;
          while ((pos != fa[p2]))
          {
            a[cnt].push_back(pos);
            pos = fa[pos];
          }
        } else if (((c3 > 3) && ((c3 % 3) != 0)))
        {
          cnt += 1;
          pos = p1;
          while ((pos != fa[p2]))
          {
            a[cnt].push_back(pos);
            pos = fa[pos];
          }
          a[cnt].push_back(i);
        }
      }
      if ((cnt == k))
      {
        break;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= cnt))
    {
      printf("%d\n", a[i].size());
      {
        var j = 0;
        while ((j < a[i].size()))
        {
          printf("%d ", a[i][j]);
          j += 1;
        }
      }
      printf("\n");
      i += 1;
    }
  }
  return 0;
}
