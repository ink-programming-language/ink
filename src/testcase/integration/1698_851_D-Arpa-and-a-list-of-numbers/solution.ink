// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var X: dynamic = cpp_uninitialized();

var Y: dynamic = cpp_uninitialized();

var A: dynamic = [0];

var B: dynamic = [0];

func cnt(l: dynamic, r: dynamic) -> dynamic
{
  r = min(r, (1000007 - 1));
  if ((l > r))
  {
    return 0;
  }
  return (A[r] - A[(l - 1)]);
}

func sum(l: dynamic, r: dynamic) -> dynamic
{
  r = min(r, (1000007 - 1));
  if ((l > r))
  {
    return 0;
  }
  return (B[r] - B[(l - 1)]);
}

func check(k: dynamic) -> dynamic
{
  var res: dynamic = 0;
  var d: dynamic = (X / Y);
  d = min(k, d);
  {
    var i: dynamic = 1;
    while ((i < 1000007))
    {
      var l: dynamic = i;
      var r: dynamic = ((l + k) - 1);
      var mid: dynamic = (r - d);
      mid = max(mid, l);
      res += ((((cnt(mid, r) * cpp_cast(r)) - sum(mid, r))) * cpp_cast(Y));
      res += (cnt(l, (mid - 1)) * cpp_cast(X));
      i += k;
    }
  }
  return res;
}

func main() -> dynamic
{
  scanf("%d%d%d", (&n), (&X), (&Y));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var a: dynamic = cpp_uninitialized();
      scanf("%d", (&a));
      A[a] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < 1000007))
    {
      B[i] = ((A[i] * cpp_cast(i)) + B[(i - 1)]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < 1000007))
    {
      A[i] += A[(i - 1)];
      i += 1;
    }
  }
  var ans: dynamic = 0x7FFFFFFFFFFFFFFF;
  {
    var i: dynamic = 2;
    while ((i < 1000007))
    {
      ans = min(ans, check(i));
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
