// Translated from solution.cpp.

var eps = 1e-8;

var inf = 1e20;

var pi = acos(-1.0);

var maxn = (1e6 + 7);

var vis = cpp_array(maxn, 2);

func gcd(a: dynamic, b: dynamic)
{
  return if ((b == 0)) a else gcd(b, (a % b));
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  while (((cin >> n) >> m))
  {
    if ((m < (n - 1)))
    {
      puts("Impossible");
      continue;
    }
    var k = 0;
    memset(vis, 0, cpp_sizeof((vis)));
    {
      var i = 1;
      while ((i < n))
      {
        {
          var j = (i + 1);
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
        var i = 0;
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
