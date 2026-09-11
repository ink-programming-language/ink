// Translated from solution.cpp.

var eps: dynamic = 1e-8;

var inf: dynamic = 1e20;

var pi: dynamic = acos(-1.0);

var maxn: dynamic = (1e6 + 7);

var vis: dynamic = cpp_array(maxn, 2);

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  ((b == 0)) ? a : gcd(b, (a % b));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  while (((cin >> n) >> m))
  {
    if ((m < (n - 1)))
    {
      puts("Impossible");
      continue;
    }
    var k: dynamic = 0;
    memset(vis, 0, cpp_sizeof((vis)));
    {
      var i: dynamic = 1;
      while ((i < n))
      {
        {
          var j: dynamic = (i + 1);
          while ((j <= n))
          {
            if ((gcd(i, j) == 1))
            {
              vis[0][k] = i;
              vis[1][k] = j;
              k += 1;
              if ((k > m))
              {
                break;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    if ((k < m))
    {
      puts("Impossible");
    } else
    {
      puts("Possible");
      {
        var i: dynamic = 0;
        while ((i < m))
        {
          printf("%lld %lld\n", vis[0][i], vis[1][i]);
          i += 1;
        }
      }
    }
  }
  return 0;
}
