// Translated from solution.cpp.

var a: dynamic = cpp_array(100005);

var b: dynamic = cpp_array(100005);

var ans: dynamic = cpp_array(100005);

func main() -> dynamic
{
  var mod: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var sum: dynamic = 0;
  scanf("%d %d %d", (&n), (&m), (&mod));
  {
    i = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < m))
    {
      scanf("%d", (&b[i]));
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      if ((i < m))
      {
        sum += b[i];
      }
      if ((i >= ((n - m) + 1)))
      {
        sum -= b[(i - (((n - m) + 1)))];
      }
      if ((sum < 0))
      {
        sum += mod;
      }
      sum %= mod;
      ans[i] = (((a[i] + sum)) % mod);
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      printf("%d ", ans[i]);
      i += 1;
    }
  }
}
