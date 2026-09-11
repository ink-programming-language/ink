// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var mn: dynamic = cpp_array(2005);

var a: dynamic = cpp_array(2005);

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  read(n, m);
  {
    i = 0;
    while ((i < n))
    {
      scanf("%lld", (&a[i]));
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      mn[i] = 2e9;
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      s = (i * m);
      {
        j = 0;
        while ((j < n))
        {
          mn[j] = min(mn[j], a[((((i - j) + n)) % n)]);
          s += mn[j];
          j += 1;
        }
      }
      if (((i == 0) || (s < ans)))
      {
        ans = s;
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
