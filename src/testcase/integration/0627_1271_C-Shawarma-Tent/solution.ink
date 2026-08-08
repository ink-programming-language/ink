// Translated from solution.cpp.

func xpow(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return 1;
  }
  if (((b % 2) == 0))
  {
    var k = xpow(a, (b / 2));
    return (k * k);
  }
  if (((b % 2) != 0))
  {
    return (a * xpow(a, (b - 1)));
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic;
  var sx: dynamic;
  var sy: dynamic;
  read(n, sx, sy);
  var l = 0;
  var r = 0;
  var d = 0;
  var u = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var x: dynamic;
      var y: dynamic;
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
  var maxi = max([l, r, d, u]);
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
