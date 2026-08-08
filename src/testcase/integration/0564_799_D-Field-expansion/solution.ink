// Translated from solution.cpp.

func chkmin(a: dynamic, b: dynamic)
{
  return if ((a > b)) cpp_comma(cpp_assign(a, "=", b), 1) else 0;
}

func chkmax(a: dynamic, b: dynamic)
{
  return if ((a < b)) cpp_comma(cpp_assign(a, "=", b), 1) else 0;
}

func smin(a: dynamic, b: dynamic)
{
  return if ((a > b)) cpp_assign(a, "=", b) else a;
}

func smax(a: dynamic, b: dynamic)
{
  return if ((a < b)) cpp_assign(a, "=", b) else a;
}

var N = (cpp_cast(2e5) + 5);

var mod = cpp_cast(0);

var sz = cpp_array(N);

var dp = cpp_array(N);

var odp = cpp_array(N);

func main()
{
  var a: dynamic;
  var b: dynamic;
  var h: dynamic;
  var w: dynamic;
  var n: dynamic;
  read(a, b, h, w, n);
  {
    var j = 0;
    while ((j < n))
    {
      read(sz[j]);
      j += 1;
    }
  }
  sort(sz, (sz + n));
  reverse(sz, (sz + n));
  dp[h] = 1;
  {
    var j = 0;
    while ((j <= min(n, 50)))
    {
      {
        var k = a;
        while ((k < N))
        {
          if (((dp[k] * w) >= b))
          {
            write(j, "\n");
            return 0;
          }
          k += 1;
        }
      }
      {
        var k = b;
        while ((k < N))
        {
          if (((dp[k] * w) >= a))
          {
            write(j, "\n");
            return 0;
          }
          k += 1;
        }
      }
      memcpy(odp, dp, cpp_sizeof(dp));
      memset(dp, 0, cpp_sizeof(dp));
      if ((j != n))
      {
        {
          var k = 0;
          while ((k < N))
          {
            var nxt = min((k * sz[j]), (N - 1));
            dp[k] = max(dp[k], min(N, (odp[k] * sz[j])));
            dp[nxt] = max(dp[nxt], min(N, odp[k]));
            k += 1;
          }
        }
      }
      j += 1;
    }
  }
  write(-1, "\n");
  return 0;
}
