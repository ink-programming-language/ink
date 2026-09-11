// Translated from solution.cpp.

func power(x: dynamic, n: dynamic) -> dynamic
{
  var result: dynamic = 1;
  while (n)
  {
    if (((n % 2) == 1))
    {
      result = (result * x);
    }
    n = (n / 2);
    x = (x * x);
  }
  return result;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((!b))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return (((a * b)) / gcd(a, b));
}

func BS(a: dynamic, s: dynamic, n: dynamic, val: dynamic) -> dynamic
{
  var mid: dynamic = cpp_uninitialized();
  var beg: dynamic = s;
  var end: dynamic = (n - 1);
  while ((beg <= end))
  {
    mid = (((beg + end)) / 2);
    if ((val == a[mid]))
    {
      break;
    } else if ((val > a[mid]))
    {
      beg = (mid + 1);
    } else
    {
      end = (mid - 1);
    }
  }
  return mid;
}

func mul(x: dynamic, y: dynamic, m: dynamic) -> dynamic
{
  var z: dynamic = ((1 * x) * y);
  if ((z >= m))
  {
    z %= m;
  }
  return z;
}

func powmod(x: dynamic, y: dynamic, m: dynamic) -> dynamic
{
  var r: dynamic = 1;
  while (y)
  {
    if ((y & 1))
    {
      r = mul(r, x, m);
    }
    y >>= 1;
    x = mul(x, x, m);
  }
  return r;
}

func start() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
}

func main() -> dynamic
{
  start();
  var t: dynamic = cpp_uninitialized();
  read((t));
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    var s: dynamic = cpp_uninitialized();
    read(s);
    var c: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i < n))
      {
        if ((s[i] == s[(i - 1)]))
        {
          c += 1;
        }
        i += 1;
      }
    }
    write((((c + 1)) / 2), "\n");
  }
  return 0;
}
