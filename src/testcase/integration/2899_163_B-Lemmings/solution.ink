// Translated from solution.cpp.

var inf = 0x3f3f3f3f;

var mod = (1e9 + 7);

var maxn = (1e5 + 10);

var eps = 1e-8;

class node
{
  var w: dynamic;
  var v: dynamic;
  var id: dynamic;
  func operator_less(p: dynamic)
  {
      if ((w != p.w))
      {
        return (w < p.w);
      } else
      {
        return (v < p.v);
      }
    }
}

var a = cpp_array(maxn);

var n: dynamic;

var k: dynamic;

var h: dynamic;

var use = cpp_array(maxn);

func check(t: dynamic)
{
  var now = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((((t * a[i].v) / h) >= now))
      {
        use[now] = a[i].id;
        now += 1;
      }
      i += 1;
    }
  }
  return (now > k);
}

var ans = cpp_array(maxn);

func main()
{
  scanf("%d%d%d", (&n), (&k), (&h));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i].w));
      a[i].id = i;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i].v));
      i += 1;
    }
  }
  sort((a + 1), ((a + 1) + n));
  var l = 0;
  var r = 1e10;
  {
    var i = 1;
    while ((i <= 200))
    {
      var mid = (((r + l)) / 2);
      if (check(mid))
      {
        r = mid;
        {
          var i = 1;
          while ((i <= k))
          {
            ans[i] = use[i];
            i += 1;
          }
        }
      } else
      {
        l = mid;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= k))
    {
      printf("%d ", ans[i]);
      i += 1;
    }
  }
  return 0;
}
