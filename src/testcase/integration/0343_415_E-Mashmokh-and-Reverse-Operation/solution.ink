// Translated from solution.cpp.

var xx: dynamic = [0, 0, 1, -1];

var yy: dynamic = [1, -1, 0, 0];

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(int_cpp((1048576 + 2000)));

var b: dynamic = cpp_array(int_cpp((1048576 + 2000)));

var f: dynamic = cpp_array(2, 30);

var res: dynamic = 0;

func build(l: dynamic, r: dynamic, h: dynamic) -> dynamic
{
  if ((l == r))
  {
    return;
  }
  var mid: dynamic = (((l + r)) / 2);
  build(l, mid, (h + 1));
  build((mid + 1), r, (h + 1));
  var i: dynamic = l;
  var j: dynamic = (mid + 1);
  var k: dynamic = l;
  while (((i <= mid) && (j <= r)))
  {
    if ((a[i] <= a[j]))
    {
      b[k] = a[i];
      i += 1;
      f[h][1] += (j - ((mid + 1)));
    } else
    {
      b[k] = a[j];
      j += 1;
      f[h][0] += ((i - l));
    }
    k += 1;
  }
  while ((i <= mid))
  {
    b[k] = a[i];
    i += 1;
    k += 1;
    f[h][1] += (r - mid);
  }
  while ((j <= r))
  {
    b[k] = a[j];
    j += 1;
    k += 1;
    f[h][0] += (((mid - l) + 1));
  }
  j = mid;
  var d: dynamic = 0;
  {
    var i: dynamic = (l);
    var b: dynamic = (mid);
    while ((i <= b))
    {
      if (((i == l) || (a[i] != a[(i - 1)])))
      {
        d = 0;
      }
      while ((((j + 1) <= r) && (a[(j + 1)] <= a[i])))
      {
        j += 1;
        if ((a[j] == a[i]))
        {
          d += 1;
        }
      }
      f[h][0] -= d;
      i += 1;
    }
  }
  {
    var i: dynamic = (l);
    var b: dynamic = (r);
    while ((i <= b))
    {
      a[i] = b[i];
      i += 1;
    }
  }
}

func solve(x: dynamic) -> dynamic
{
  {
    var i: dynamic = (x);
    var b: dynamic = (n);
    while ((i <= b))
    {
      res -= f[i][1];
      swap(f[i][1], f[i][0]);
      res += f[i][1];
      i += 1;
    }
  }
  printf("%I64d\n", res);
}

func main() -> dynamic
{
  scanf("%d", (&n));
  m = ((1 << n));
  {
    var i: dynamic = (1);
    var b: dynamic = (m);
    while ((i <= b))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  build(1, m, 0);
  {
    var i: dynamic = (0);
    var b: dynamic = (n);
    while ((i <= b))
    {
      res += f[i][1];
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  scanf("%d", (&q));
  {
    var i: dynamic = (1);
    var b: dynamic = (q);
    while ((i <= b))
    {
      var x: dynamic = cpp_uninitialized();
      scanf("%d", (&x));
      solve((n - x));
      i += 1;
    }
  }
}
