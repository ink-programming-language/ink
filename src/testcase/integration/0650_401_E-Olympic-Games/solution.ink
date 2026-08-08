// Translated from solution.cpp.

var num = cpp_array(100005);

var d = cpp_array(6, 100005);

var n: dynamic;

var m: dynamic;

var l: dynamic;

var r: dynamic;

var p: dynamic;

func cal(ql: dynamic, qh: dynamic, mm: dynamic)
{
  qh /= mm;
  ql = ((((ql + mm) - 1)) / mm);
  return (((((((qh - ql) + 1)) * ((n + 1))) - (mm * (((((qh * ((qh + 1))) - (ql * ((ql - 1))))) / 2))))) % p);
}

func main()
{
  var ans = 0;
  read(n, m, l, r, p);
  {
    var i = 2;
    while ((i <= m))
    {
      if ((num[i] == 0))
      {
        {
          var j = i;
          while ((j <= m))
          {
            d[j][cpp_update(num[j], "++")] = i;
            j += i;
          }
        }
      }
      i += 1;
    }
  }
  var lo = l;
  var hi = r;
  var mmin = min(m, r);
  {
    var w = 1;
    while ((w <= mmin))
    {
      while (((lo > 1) && (((l * l) - (w * w)) <= (((lo - 1)) * ((lo - 1))))))
      {
        lo -= 1;
      }
      while ((((r * r) - (w * w)) < (hi * hi)))
      {
        hi -= 1;
      }
      if (((lo <= hi) && (lo <= n)))
      {
        var t = ((1 << num[w]));
        var a = 0;
        {
          var i = 0;
          while ((i < t))
          {
            var ii = i;
            var p1 = 1;
            var p2 = 1;
            {
              var j = 0;
              while ((j < num[w]))
              {
                if ((ii & 1))
                {
                  p1 *= d[w][j];
                  p2 *= -1;
                }
                ii >>= 1;
                j += 1;
              }
            }
            a += (p2 * cal(lo, if ((hi < n)) hi else n, p1));
            i += 1;
          }
        }
        ans = (((ans + ((((m - w) + 1)) * a))) % p);
        if ((ans < 0))
        {
          ans += p;
        }
      }
      w += 1;
    }
  }
  if (((l <= 1) && (r >= 1)))
  {
    ans = (((((ans * 2) + (n * ((m + 1)))) + (m * ((n + 1))))) % p);
  } else
  {
    ans = (((ans * 2)) % p);
  }
  write(ans, "\n");
  return 0;
}
