// Translated from solution.cpp.

func chkmin(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a > b)) ? cpp_comma(cpp_assign(a, "=", b), 1) : 0;
}

func chkmax(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a < b)) ? cpp_comma(cpp_assign(a, "=", b), 1) : 0;
}

func smin(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a > b)) ? cpp_assign(a, "=", b) : a;
}

func smax(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a < b)) ? cpp_assign(a, "=", b) : a;
}

var N: dynamic = (cpp_cast(2e5) + 5);

var mod: dynamic = cpp_cast(0);

var sz: dynamic = cpp_array(N);

var dp: dynamic = cpp_array(N);

var odp: dynamic = cpp_array(N);

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(a, b, h, w, n);
  {
    var j: dynamic = 0;
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
    var j: dynamic = 0;
    while ((j <= min(n, 50)))
    {
      {
        var k: dynamic = a;
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
        var k: dynamic = b;
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
          var k: dynamic = 0;
          while ((k < N))
          {
            var nxt: dynamic = min((k * sz[j]), (N - 1));
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
