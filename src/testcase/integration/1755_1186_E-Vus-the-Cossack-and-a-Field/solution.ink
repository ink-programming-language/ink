// Translated from solution.cpp.

var maxn: dynamic = 1010;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn, maxn);

var s: dynamic = cpp_array(maxn, maxn);

var xa: dynamic = cpp_uninitialized();

var ya: dynamic = cpp_uninitialized();

var xb: dynamic = cpp_uninitialized();

var yb: dynamic = cpp_uninitialized();

var ch: dynamic = cpp_uninitialized();

func bitcnt(x: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  while (x)
  {
    if ((x & 1))
    {
      ret += 1;
    }
    x >>= 1;
  }
  return ret;
}

func rev(x: dynamic, y: dynamic) -> dynamic
{
  x -= 1;
  y -= 1;
  return (((bitcnt(x) + bitcnt(y))) & 1);
}

func sum(x: dynamic, y: dynamic) -> dynamic
{
  if (((x == 0) || (y == 0)))
  {
    return 0;
  }
  var ret: dynamic = 0;
  var fx: dynamic = ((((x + n) - 1)) / n);
  var fy: dynamic = ((((y + m) - 1)) / m);
  ret += ((((((fx - 1)) * ((fy - 1))) / 2) * n) * m);
  if ((((fx % 2) == 0) && ((fy % 2) == 0)))
  {
    if (rev((fx - 1), (fy - 1)))
    {
      ret += ((n * m) - s[n][m]);
    } else
    {
      ret += s[n][m];
    }
  }
  ret += (((((fx - 1)) / 2) * n) * ((y - (((fy - 1)) * m))));
  if (((fx % 2) == 0))
  {
    if (rev((fx - 1), fy))
    {
      ret += ((n * ((y - (((fy - 1)) * m)))) - s[n][(y - (((fy - 1)) * m))]);
    } else
    {
      ret += s[n][(y - (((fy - 1)) * m))];
    }
  }
  ret += (((((fy - 1)) / 2) * ((x - (((fx - 1)) * n)))) * m);
  if (((fy % 2) == 0))
  {
    if (rev(fx, (fy - 1)))
    {
      ret += ((((x - (((fx - 1)) * n))) * m) - s[(x - (((fx - 1)) * n))][m]);
    } else
    {
      ret += s[(x - (((fx - 1)) * n))][m];
    }
  }
  if (rev(fx, fy))
  {
    ret += ((((x - (((fx - 1)) * n))) * ((y - (((fy - 1)) * m)))) - s[(x - (((fx - 1)) * n))][(y - (((fy - 1)) * m))]);
  } else
  {
    ret += s[(x - (((fx - 1)) * n))][(y - (((fy - 1)) * m))];
  }
  return ret;
}

func main() -> dynamic
{
  scanf("%lld%lld%lld", (&n), (&m), (&q));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          scanf(" %c", (&ch));
          a[i][j] = (ch - cpp_char("0"));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          s[i][j] = (((s[(i - 1)][j] + s[i][(j - 1)]) - s[(i - 1)][(j - 1)]) + a[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  while (cpp_update(q, "--"))
  {
    scanf("%lld%lld%lld%lld", (&xa), (&ya), (&xb), (&yb));
    printf("%lld\n", (((sum((xa - 1), (ya - 1)) + sum(xb, yb)) - sum((xa - 1), yb)) - sum(xb, (ya - 1))));
  }
  return 0;
}
