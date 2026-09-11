// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

class node
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

var d: dynamic = cpp_array(100005);

func cmp(x: dynamic, y: dynamic) -> dynamic
{
  return (x.x < y.x);
}

var a: dynamic = cpp_array(15, 100005);

func chk(x: dynamic) -> dynamic
{
  {
    var i: dynamic = 1;
    var j: dynamic = 1;
    while ((i <= n))
    {
      while (((j < n) && ((d[(j + 1)].x - d[i].x) <= x)))
      {
        j += 1;
      }
      if (((max(a[(i - 1)][1], a[(j + 1)][3]) - min(a[(i - 1)][2], a[(j + 1)][4])) <= x))
      {
        return 1;
      }
      i += 1;
    }
  }
  return 0;
}

func main() -> dynamic
{
  scanf("%lld", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%lld%lld", (&x), (&y));
      d[i].x = (x - y);
      d[i].y = (x + y);
      i += 1;
    }
  }
  sort((d + 1), ((d + n) + 1), cmp);
  a[0][1] = ((-1) << 40);
  a[0][2] = (1 << 40);
  a[(n + 1)][3] = ((-1) << 40);
  a[(n + 1)][4] = (1 << 40);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i][1] = max(a[(i - 1)][1], d[i].y);
      a[i][2] = min(a[(i - 1)][2], d[i].y);
      a[((n - i) + 1)][3] = max(a[((n - i) + 2)][3], d[((n - i) + 1)].y);
      a[((n - i) + 1)][4] = min(a[((n - i) + 2)][4], d[((n - i) + 1)].y);
      i += 1;
    }
  }
  var l: dynamic = 0;
  var r: dynamic = (1 << 40);
  var mid: dynamic = cpp_uninitialized();
  while ((l <= r))
  {
    mid = (((l + r)) >> 1);
    if (chk(mid))
    {
      r = (mid - 1);
    } else
    {
      l = (mid + 1);
    }
  }
  printf("%.6lf", (l / 2.0));
  return 0;
}
