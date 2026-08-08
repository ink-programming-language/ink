// Translated from solution.cpp.

var N: dynamic;

var M: dynamic;

var a = cpp_array(300005);

var p = cpp_array(300005);

var diff: dynamic;

var k: dynamic;

func gcd(x: dynamic, y: dynamic)
{
  if ((y == 0))
  {
    return x;
  }
  return gcd(y, (x % y));
}

func main()
{
  read(N, M);
  {
    var i = 0;
    while ((i < N))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < M))
    {
      read(p[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < N))
    {
      diff = (a[i] - a[(i - 1)]);
      k = gcd(k, diff);
      i += 1;
    }
  }
  {
    var i = 0;
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
