// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

func input() -> dynamic
{
  read(n);
  return 0;
}

func gcd(p: dynamic, q: dynamic) -> dynamic
{
  var r: dynamic = cpp_uninitialized();
  if ((p < q))
  {
    var t: dynamic = p;
    p = q;
    q = t;
  }
  while (1)
  {
    r = (p % q);
    if ((r == 0))
    {
      break;
    }
    p = q;
    q = r;
  }
  return q;
}

func f(a: dynamic, b: dynamic) -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  if ((gcd(a, b) != 1))
  {
    return -1;
  }
  {
    var i: dynamic = 0;
    while (true)
    {
      if ((a < b))
      {
        var t: dynamic = a;
        a = b;
        b = t;
      }
      if (((a == 2) && (b == 1)))
      {
        return (1 + i);
      }
      if (((a == 1) && (b == 1)))
      {
        return i;
      }
      if (((a < 1) || (b < 1)))
      {
        return -1;
      }
      if ((a == b))
      {
        return -1;
      }
      if ((b == 1))
      {
        return (((a - 1)) + i);
      }
      t = (a - b);
      a = b;
      b = t;
      i += 1;
    }
  }
}

func solve() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var min: dynamic = -1;
  var x: dynamic = cpp_uninitialized();
  if ((n == 1))
  {
    return 0;
  }
  {
    i = (n / 2);
    while (i)
    {
      x = f(n, i);
      if ((x == -1))
      {
        i -= 1;
        continue;
      }
      if (((min == -1) || (x < min)))
      {
        min = x;
      }
      i -= 1;
    }
  }
  return min;
}

func main() -> dynamic
{
  input();
  write(solve(), "\n");
  return 0;
}
