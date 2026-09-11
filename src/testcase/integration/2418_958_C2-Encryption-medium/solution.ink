// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(55, 105);

func modd(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    return (x + p);
  } else if ((x >= p))
  {
    return (x - p);
  }
  return x;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  cerr.tie(null);
  read(n, k, p);
  memset(dp, 128, cpp_sizeof(dp));
  dp[0][0] = 0;
  var z: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      z = (((z + x)) % p);
      var vz: dynamic = cpp_construct(55, -2e9);
      {
        var l: dynamic = 0;
        while ((l < k))
        {
          {
            var j: dynamic = 0;
            while ((j < p))
            {
              vz[(l + 1)] = max(vz[(l + 1)], (dp[j][l] + modd(((z - j) + p))));
              j += 1;
            }
          }
          l += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < 55))
        {
          dp[z][i] = max(dp[z][i], vz[i]);
          i += 1;
        }
      }
      i += 1;
    }
  }
  write(dp[z][k], cpp_char("\n"));
}
