// Translated from solution.cpp.

var maxn = 200003;

var INF = 0x3f3f3f3f3f3f3f3f;

var n: dynamic;

var Q: dynamic;

var pos = cpp_array(maxn);

var a = cpp_array(maxn);

func main()
{
  scanf("%d%d", (&n), (&Q));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a[i].first));
      a[i].second = i;
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1));
  a[0].first = (-INF);
  a[(n + 1)].first = INF;
  {
    var i = 1;
    while ((i <= n))
    {
      pos[a[i].second] = i;
      i += 1;
    }
  }
  while (cpp_update(Q, "--"))
  {
    var y: dynamic;
    var p: dynamic;
    var l: dynamic;
    scanf("%d%lld", (&y), (&l));
    y = pos[y];
    var dir = 1;
    var flag = 0;
    while (1)
    {
      if ((flag == 2))
      {
        printf("%d\n", a[p].second);
        break;
      }
      if (dir)
      {
        p = ((upper_bound((a + 1), ((a + n) + 1), pair((a[y].first + l), maxn)) - a) - 1);
        var len = (a[p].first - a[y].first);
        if ((p == y))
        {
          flag += 1;
          dir = 0;
          continue;
        }
        flag = 0;
        if (((l < (a[(p + 1)].first - a[y].first)) && (((a[y].first + len) - l) > a[(y - 1)].first)))
        {
          if (((l / len) % 2))
          {
            dir = 0;
            y = p;
          }
          l %= len;
        } else
        {
          l -= len;
          y = p;
          dir = 0;
        }
      } else
      {
        p = (lower_bound((a + 1), ((a + n) + 1), pair((a[y].first - l), 0)) - a);
        var len = (a[y].first - a[p].first);
        if ((p == y))
        {
          flag += 1;
          dir = 1;
          continue;
        }
        flag = 0;
        if (((l > a[(p - 1)].first) && (((a[y].first - len) + l) < a[(y + 1)].first)))
        {
          if (((l / len) % 2))
          {
            dir = 0;
            y = p;
          }
          l %= len;
        } else
        {
          l -= len;
          y = p;
          dir = 1;
        }
      }
    }
  }
  return 0;
}
