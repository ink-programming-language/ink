// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var p: dynamic;

var dp = cpp_array(55, 105);

func modd(x: dynamic)
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  cerr.tie(null);
  read(n, k, p);
  memset(dp, 128, cpp_sizeof(dp));
  dp[0][0] = 0;
  var z = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      read(x);
      z = (((z + x)) % p);
      var vz = cpp_construct(55, -2e9);
      {
        var l = 0;
        while ((l < k))
        {
          {
            var j = 0;
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
        var i = 0;
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
