// Translated from solution.cpp.

var maxn: dynamic = (5 * 100005);

var a: dynamic = cpp_array(maxn);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var lim: dynamic = cpp_uninitialized();

var head: dynamic = cpp_array(maxn);

var Next: dynamic = cpp_array((maxn * 2));

var to: dynamic = cpp_array((maxn * 2));

var tot: dynamic = 0;

var d: dynamic = cpp_array(maxn);

var fa: dynamic = cpp_array(maxn);

var size: dynamic = cpp_array(maxn);

var vis: dynamic = cpp_array(maxn);

func add(x: dynamic, y: dynamic) -> dynamic
{
  to[cpp_update(tot, "++")] = y;
  Next[tot] = head[x];
  head[x] = tot;
}

func dfs_tree(x: dynamic, f: dynamic) -> dynamic
{
  size[x] = 1;
  {
    var i: dynamic = head[x];
    while (i)
    {
      var y: dynamic = to[i];
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

func main() -> dynamic
{
  scanf("%d%d%d", (&n), (&m), (&k));
  lim = ceil(((cpp_cast(n) * 1.0) / k));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%d%d", (&x), (&y));
      add(x, y);
      add(y, x);
      i += 1;
    }
  }
  vis[1] = 1;
  d[1] = 1;
  dfs_tree(1, 0);
  var pos: dynamic = 0;
  {
    var i: dynamic = 1;
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
  var cnt: dynamic = 0;
  puts("CYCLES");
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((size[i] == 1))
      {
        var p1: dynamic = 0;
        var p2: dynamic = 0;
        {
          var j: dynamic = head[i];
          while (j)
          {
            var y: dynamic = to[j];
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
        var c1: dynamic = ((d[i] - d[p1]) + 1);
        var c2: dynamic = ((d[i] - d[p2]) + 1);
        var c3: dynamic = ((d[p1] - d[p2]) + 2);
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
    var i: dynamic = 1;
    while ((i <= cnt))
    {
      printf("%d\n", a[i].size());
      {
        var j: dynamic = 0;
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
