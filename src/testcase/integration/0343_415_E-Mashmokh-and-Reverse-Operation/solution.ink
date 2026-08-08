// Translated from solution.cpp.

var xx = [0, 0, 1, -1];

var yy = [1, -1, 0, 0];

var n: dynamic;

var m: dynamic;

var a = cpp_array(int_cpp((1048576 + 2000)));

var b = cpp_array(int_cpp((1048576 + 2000)));

var f = cpp_array(2, 30);

var res = 0;

func build(l: dynamic, r: dynamic, h: dynamic)
{
  if ((l == r))
  {
    return;
  }
  var mid = (((l + r)) / 2);
  build(l, mid, (h + 1));
  build((mid + 1), r, (h + 1));
  var i = l;
  var j = (mid + 1);
  var k = l;
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
  var d = 0;
  {
    var i = (l);
    var b = (mid);
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
    var i = (l);
    var b = (r);
    while ((i <= b))
    {
      a[i] = b[i];
      i += 1;
    }
  }
}

func solve(x: dynamic)
{
  {
    var i = (x);
    var b = (n);
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

func main()
{
  scanf("%d", (&n));
  m = ((1 << n));
  {
    var i = (1);
    var b = (m);
    while ((i <= b))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  build(1, m, 0);
  {
    var i = (0);
    var b = (n);
    while ((i <= b))
    {
      res += f[i][1];
      i += 1;
    }
  }
  var q: dynamic;
  scanf("%d", (&q));
  {
    var i = (1);
    var b = (q);
    while ((i <= b))
    {
      var x: dynamic;
      scanf("%d", (&x));
      solve((n - x));
      i += 1;
    }
  }
}
