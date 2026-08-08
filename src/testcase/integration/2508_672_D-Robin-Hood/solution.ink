// Translated from solution.cpp.

func max(a: dynamic, b: dynamic)
{
  return if ((a > b)) a else b;
}

var maxn = 500005;

var a = cpp_array(maxn);

var n: dynamic;

var k: dynamic;

var S: dynamic;

var s: dynamic;

var d: dynamic;

var D: dynamic;

var p: dynamic;

var L: dynamic;

var R: dynamic;

func t_main()
{
  scanf("%d %d", (&n), (&k));
  {
    var i = 0;
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
    var i = 0;
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
    var i = 0;
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
    var i = (n - 1);
    while ((i > 0))
    {
      var N = (n - i);
      var delta = (a[i] - a[(i - 1)]);
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
    var i = 0;
    while ((i < (n - 1)))
    {
      var N = (i + 1);
      var delta = (a[(i + 1)] - a[i]);
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

func run()
{
  var t = 1;
  while (cpp_update(t, "--"))
  {
    t_main();
  }
}

func main()
{
  run();
  return 0;
}
