// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((b > a))
  {
    return gcd(b, a);
  }
  return  ((b == 0)) ? a : gcd(b, (a % b));
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    var a: dynamic = cpp_array((n + 1));
    {
      var i: dynamic = 1;
      while ((i < (n + 1)))
      {
        read(a[i]);
        i += 1;
      }
    }
    var count: dynamic = cpp_uninitialized();
    var ans: dynamic = n;
    var d: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i < (n + 1)))
      {
        if (count[a[i]])
        {
          ans = min(ans, ((i - count[a[i]]) + 1));
          d = 1;
        }
        count[a[i]] = i;
        i += 1;
      }
    }
    if (((ans == 1) || (d == 0)))
    {
      write(-1, "\n");
    } else
    {
      write(ans, "\n");
    }
  }
  return 0;
}
