// Translated from solution.cpp.

var maxn = (1e6 + 7);

var INF = (1e18 + 7);

var a = cpp_array(maxn);

var aa = cpp_array(maxn);

var pre = cpp_array(maxn);

var b = cpp_array(maxn);

func main()
{
  var i: dynamic;
  var j: dynamic;
  var m: dynamic;
  var n: dynamic;
  var t: dynamic;
  var z: dynamic;
  var k: dynamic;
  scanf("%d%d%d", (&n), (&m), (&k));
  {
    i = 1;
    while ((i <= m))
    {
      scanf("%d", (&a[i]));
      aa[a[i]] = 1;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= k))
    {
      scanf("%d", (&b[i]));
      i += 1;
    }
  }
  if (aa[0])
  {
    puts("-1");
    return 0;
  }
  {
    i = 0;
    while ((i < n))
    {
      if ((!aa[i]))
      {
        pre[i] = i;
      } else
      {
        pre[i] = pre[(i - 1)];
      }
      i += 1;
    }
  }
  var ans = 1e18;
  {
    i = 1;
    while ((i <= k))
    {
      var tem = 0;
      var cnt = 0;
      while ((cnt < n))
      {
        if (((cnt + i) >= n))
        {
          tem += 1;
          cnt = n;
          break;
        }
        if ((pre[(cnt + i)] <= cnt))
        {
          break;
        } else
        {
          cnt = pre[(cnt + i)];
          tem += 1;
        }
      }
      if ((cnt == n))
      {
        ans = min(ans, (b[i] * tem));
      }
      i += 1;
    }
  }
  if ((ans != INF))
  {
    printf("%lld\n", ans);
  } else
  {
    printf("-1\n");
  }
  return 0;
}
