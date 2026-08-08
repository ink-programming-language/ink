// Translated from solution.cpp.

var n: dynamic;

func input()
{
  read(n);
  return 0;
}

func gcd(p: dynamic, q: dynamic)
{
  var r: dynamic;
  if ((p < q))
  {
    var t = p;
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

func f(a: dynamic, b: dynamic)
{
  var t: dynamic;
  if ((gcd(a, b) != 1))
  {
    return -1;
  }
  {
    var i = 0;
    while (true)
    {
      if ((a < b))
      {
        var t = a;
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

func solve()
{
  var i: dynamic;
  var min = -1;
  var x: dynamic;
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

func main()
{
  input();
  write(solve(), "\n");
  return 0;
}
