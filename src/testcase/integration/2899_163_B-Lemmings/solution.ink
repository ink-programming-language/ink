// Translated from solution.cpp.

var inf: dynamic = 0x3f3f3f3f;

var mod: dynamic = (1e9 + 7);

var maxn: dynamic = (1e5 + 10);

var eps: dynamic = 1e-8;

class node
{
  var w: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
  func operator_less(p: dynamic) -> dynamic
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

var a: dynamic = cpp_array(maxn);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var h: dynamic = cpp_uninitialized();

var use: dynamic = cpp_array(maxn);

func check(t: dynamic) -> dynamic
{
  var now: dynamic = 1;
  {
    var i: dynamic = 1;
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

var ans: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  scanf("%d%d%d", (&n), (&k), (&h));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i].w));
      a[i].id = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i].v));
      i += 1;
    }
  }
  sort((a + 1), ((a + 1) + n));
  var l: dynamic = 0;
  var r: dynamic = 1e10;
  {
    var i: dynamic = 1;
    while ((i <= 200))
    {
      var mid: dynamic = (((r + l)) / 2);
      if (check(mid))
      {
        r = mid;
        {
          var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= k))
    {
      printf("%d ", ans[i]);
      i += 1;
    }
  }
  return 0;
}
