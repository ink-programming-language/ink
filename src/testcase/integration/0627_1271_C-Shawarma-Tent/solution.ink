// Translated from solution.cpp.

func xpow(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return 1;
  }
  if (((b % 2) == 0))
  {
    var k: dynamic = xpow(a, (b / 2));
    return (k * k);
  }
  if (((b % 2) != 0))
  {
    return (a * xpow(a, (b - 1)));
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic = cpp_uninitialized();
  var sx: dynamic = cpp_uninitialized();
  var sy: dynamic = cpp_uninitialized();
  read(n, sx, sy);
  var l: dynamic = 0;
  var r: dynamic = 0;
  var d: dynamic = 0;
  var u: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      if ((x > sx))
      {
        r += 1;
      }
      if ((x < sx))
      {
        l += 1;
      }
      if ((y > sy))
      {
        u += 1;
      }
      if ((y < sy))
      {
        d += 1;
      }
      i += 1;
    }
  }
  var maxi: dynamic = max([l, r, d, u]);
  write(maxi, "\n");
  if ((l == maxi))
  {
    sx -= 1;
  } else if ((r == maxi))
  {
    sx += 1;
  } else if ((u == maxi))
  {
    sy += 1;
  } else if ((d == maxi))
  {
    sy -= 1;
  }
  write(sx, " ", sy, "\n");
  return 0;
}
