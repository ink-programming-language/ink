// Translated from solution.cpp.

var MOD: dynamic = (1e9 + 7);

var maxn: dynamic = (1e6 + 5);

var inf: dynamic = 0x3f3f3f3f;

var INF: dynamic = 0x3f3f3f3f3f3f3f3f;

var cnt: dynamic = cpp_array(maxn);

var d: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var ma: dynamic = 0;
  var f: dynamic = 0;
  scanf("%d", (&t));
  {
    var i: dynamic = 1;
    while ((i <= t))
    {
      scanf("%d", (&x));
      cnt[x] += 1;
      ma = max(ma, x);
      i += 1;
    }
  }
  if ((cnt[0] != 1))
  {
    return cpp_comma(puts("-1"), 0);
  }
  x = 1;
  {
    var i: dynamic = 1;
    while ((i <= ma))
    {
      if ((cnt[i] != (4 * i)))
      {
        x = i;
        break;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= t))
    {
      if (((t % i) == 0))
      {
        n = i;
        m = (t / i);
        y = (((n + m) - ma) - x);
        memset(d, 0, cpp_sizeof((d)));
        {
          var j: dynamic = 1;
          while ((j <= n))
          {
            {
              var k: dynamic = 1;
              while ((k <= m))
              {
                d[(abs((x - j)) + abs((y - k)))] += 1;
                k += 1;
              }
            }
            j += 1;
          }
        }
        f = 1;
        {
          var j: dynamic = 1;
          while ((j <= ma))
          {
            if ((cnt[j] != d[j]))
            {
              f = 0;
              break;
            }
            j += 1;
          }
        }
        if (f)
        {
          return cpp_comma(printf("%d %d\n%d %d\n", n, m, x, y), 0);
        }
      }
      i += 1;
    }
  }
  puts("-1");
  return 0;
}
