// Translated from solution.cpp.

var num: dynamic = cpp_array(100005);

var d: dynamic = cpp_array(6, 100005);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

func cal(ql: dynamic, qh: dynamic, mm: dynamic) -> dynamic
{
  qh /= mm;
  ql = ((((ql + mm) - 1)) / mm);
  return (((((((qh - ql) + 1)) * ((n + 1))) - (mm * (((((qh * ((qh + 1))) - (ql * ((ql - 1))))) / 2))))) % p);
}

func main() -> dynamic
{
  var ans: dynamic = 0;
  read(n, m, l, r, p);
  {
    var i: dynamic = 2;
    while ((i <= m))
    {
      if ((num[i] == 0))
      {
        {
          var j: dynamic = i;
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
  var lo: dynamic = l;
  var hi: dynamic = r;
  var mmin: dynamic = min(m, r);
  {
    var w: dynamic = 1;
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
        var t: dynamic = ((1 << num[w]));
        var a: dynamic = 0;
        {
          var i: dynamic = 0;
          while ((i < t))
          {
            var ii: dynamic = i;
            var p1: dynamic = 1;
            var p2: dynamic = 1;
            {
              var j: dynamic = 0;
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
            a += (p2 * cal(lo,  ((hi < n)) ? hi : n, p1));
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
