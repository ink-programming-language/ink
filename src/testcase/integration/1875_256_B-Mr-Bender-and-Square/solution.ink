// Translated from solution.cpp.

var eps: dynamic = 1e-7;

var PI: dynamic = acos(-1.0);

var oo: dynamic = (1 << 29);

var N: dynamic = 101111;

var n: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

func calc(m: dynamic) -> dynamic
{
  var d: dynamic = [(x - 1), (y - 1), (n - x), (n - y)];
  var ret: dynamic = (((m * ((m + 1))) * 2) + 1);
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      if ((m <= d[i]))
      {
        i += 1;
        continue;
      }
      var t: dynamic = (m - d[i]);
      ret -= (t * t);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      {
        var j: dynamic = 1;
        while ((j < 4))
        {
          var t: dynamic = (((m - d[i]) - d[j]) - 1);
          if ((t <= 0))
          {
            j += 2;
            continue;
          }
          ret += ((t * ((t + 1))) / 2);
          j += 2;
        }
      }
      i += 2;
    }
  }
  return ret;
}

func main() -> dynamic
{
  scanf("%d%d%d%d", (&n), (&x), (&y), (&c));
  var l: dynamic = 0;
  var r: dynamic = (2 * 1000000000);
  while ((l < r))
  {
    var m: dynamic = (((l + r)) / 2);
    if ((calc(m) >= c))
    {
      r = m;
    } else
    {
      l = (m + 1);
    }
  }
  write(r, "\n");
  return 0;
}
