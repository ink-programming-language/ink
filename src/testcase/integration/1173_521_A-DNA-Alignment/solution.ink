// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(26);

var b: dynamic = cpp_uninitialized();

var ma: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

func bin(a: dynamic, n: dynamic) -> dynamic
{
  var ans: dynamic = 1;
  while (n)
  {
    if ((n & 1))
    {
      ans *= a;
      ans %= 1000000007;
    }
    a *= a;
    a %= 1000000007;
    n /= 2;
  }
  return ans;
}

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("\n%c", (&c));
      a[(c - 65)] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      if ((a[i] > ma))
      {
        ma = a[i];
        b = 1;
      } else if ((a[i] == ma))
      {
        b += 1;
      }
      i += 1;
    }
  }
  write(bin(b, n));
}
