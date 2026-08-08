// Translated from solution.cpp.

var n: dynamic;

var X: dynamic;

var Y: dynamic;

var A = [0];

var B = [0];

func cnt(l: dynamic, r: dynamic)
{
  r = min(r, (1000007 - 1));
  if ((l > r))
  {
    return 0;
  }
  return (A[r] - A[(l - 1)]);
}

func sum(l: dynamic, r: dynamic)
{
  r = min(r, (1000007 - 1));
  if ((l > r))
  {
    return 0;
  }
  return (B[r] - B[(l - 1)]);
}

func check(k: dynamic)
{
  var res = 0;
  var d = (X / Y);
  d = min(k, d);
  {
    var i = 1;
    while ((i < 1000007))
    {
      var l = i;
      var r = ((l + k) - 1);
      var mid = (r - d);
      mid = max(mid, l);
      res += ((((cnt(mid, r) * cpp_cast(r)) - sum(mid, r))) * cpp_cast(Y));
      res += (cnt(l, (mid - 1)) * cpp_cast(X));
      i += k;
    }
  }
  return res;
}

func main()
{
  scanf("%d%d%d", (&n), (&X), (&Y));
  {
    var i = 1;
    while ((i <= n))
    {
      var a: dynamic;
      scanf("%d", (&a));
      A[a] += 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < 1000007))
    {
      B[i] = ((A[i] * cpp_cast(i)) + B[(i - 1)]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < 1000007))
    {
      A[i] += A[(i - 1)];
      i += 1;
    }
  }
  var ans = 0x7FFFFFFFFFFFFFFF;
  {
    var i = 2;
    while ((i < 1000007))
    {
      ans = min(ans, check(i));
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
