// Translated from solution.cpp.

var eps = 1e-7;

var PI = acos(-1.0);

var oo = (1 << 29);

var N = 101111;

var n: dynamic;

var x: dynamic;

var y: dynamic;

var c: dynamic;

func calc(m: dynamic)
{
  var d = [(x - 1), (y - 1), (n - x), (n - y)];
  var ret = (((m * ((m + 1))) * 2) + 1);
  {
    var i = 0;
    while ((i < 4))
    {
      if ((m <= d[i]))
      {
        i += 1;
        continue;
      }
      var t = (m - d[i]);
      ret -= (t * t);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 4))
    {
      {
        var j = 1;
        while ((j < 4))
        {
          var t = (((m - d[i]) - d[j]) - 1);
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

func main()
{
  scanf("%d%d%d%d", (&n), (&x), (&y), (&c));
  var l = 0;
  var r = (2 * 1000000000);
  while ((l < r))
  {
    var m = (((l + r)) / 2);
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
