// Translated from solution.cpp.

var maxn: dynamic = (1e6 + 7);

var INF: dynamic = (1e18 + 7);

var a: dynamic = cpp_array(maxn);

var aa: dynamic = cpp_array(maxn);

var pre: dynamic = cpp_array(maxn);

var b: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
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
  var ans: dynamic = 1e18;
  {
    i = 1;
    while ((i <= k))
    {
      var tem: dynamic = 0;
      var cnt: dynamic = 0;
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
