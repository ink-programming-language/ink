// Translated from solution.cpp.

func max(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a > b)) ? a : b;
}

var maxn: dynamic = 500005;

var a: dynamic = cpp_array(maxn);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var S: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var D: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var L: dynamic = cpp_uninitialized();

var R: dynamic = cpp_uninitialized();

func t_main() -> dynamic
{
  scanf("%d %d", (&n), (&k));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (a + i));
      S += a[i];
      i += 1;
    }
  }
  sort(a, (a + n));
  d = (S / n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((a[i] < d))
      {
        s += (d - a[i]);
      }
      i += 1;
    }
  }
  D = 0;
  d = ((((S + n) - 1)) / n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((a[i] > d))
      {
        D += (a[i] - d);
      }
      i += 1;
    }
  }
  if ((D > s))
  {
    s = D;
  }
  if ((s <= k))
  {
    k = s;
  }
  L = a[0];
  R = a[(n - 1)];
  s = k;
  {
    var i: dynamic = (n - 1);
    while ((i > 0))
    {
      var N: dynamic = (n - i);
      var delta: dynamic = (a[i] - a[(i - 1)]);
      if ((s >= ((1 * delta) * N)))
      {
        s -= ((1 * delta) * N);
        R = a[(i - 1)];
      } else
      {
        R -= (s / N);
        break;
      }
      i -= 1;
    }
  }
  s = k;
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var N: dynamic = (i + 1);
      var delta: dynamic = (a[(i + 1)] - a[i]);
      if ((s >= ((1 * delta) * N)))
      {
        s -= ((1 * delta) * N);
        L = a[(i + 1)];
      } else
      {
        L += (s / N);
        break;
      }
      i += 1;
    }
  }
  printf("%d\n", cpp_cast(((R - L))));
}

func run() -> dynamic
{
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    t_main();
  }
}

func main() -> dynamic
{
  run();
  return 0;
}
