// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(300005);

var p: dynamic = cpp_array(300005);

var diff: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

func gcd(x: dynamic, y: dynamic) -> dynamic
{
  if ((y == 0))
  {
    return x;
  }
  return gcd(y, (x % y));
}

func main() -> dynamic
{
  read(N, M);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      read(p[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      diff = (a[i] - a[(i - 1)]);
      k = gcd(k, diff);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      if ((gcd(k, p[i]) == p[i]))
      {
        write("YES\n", a[0], " ", (i + 1), "\n");
        return 0;
      }
      i += 1;
    }
  }
  write("NO\n");
  return 0;
}
