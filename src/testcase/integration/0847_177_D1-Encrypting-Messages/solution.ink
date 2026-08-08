// Translated from solution.cpp.

var a = cpp_array(100005);

var b = cpp_array(100005);

var ans = cpp_array(100005);

func main()
{
  var mod: dynamic;
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var sum = 0;
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
