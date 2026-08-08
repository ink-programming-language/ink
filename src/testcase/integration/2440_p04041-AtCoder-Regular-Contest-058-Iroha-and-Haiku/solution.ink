// Translated from solution.cpp.

var Mod = (1e9 + 7);

var MaxN = 41;

var StateLen = ((5 + 7) + 5);

var N: dynamic;

var X: dynamic;

var Y: dynamic;

var Z: dynamic;

var p = cpp_array(MaxN);

var f = cpp_array((1 << StateLen), MaxN);

var end_state: dynamic;

func dp(i: dynamic, j: dynamic)
{
  if ((i == N))
  {
    return 0;
  }
  if ((~f[i][j]))
  {
    return f[i][j];
  }
  var res = 0;
  {
    var k = 1;
    while ((k <= 10))
    {
      var next_state = (((j << k)) | 1);
      if ((((next_state & end_state)) == end_state))
      {
        res += p[((N - 1) - i)];
      } else
      {
        res += dp((i + 1), (next_state & ((((1 << 17)) - 1))));
      }
      k += 1;
    }
  }
  return cpp_assign(f[i][j], "=", (res % Mod));
}

func main()
{
  ios_base.sync_with_stdio(false);
  memset(f, 0xff, cpp_sizeof(f));
  cin.tie(0);
  read(N, X, Y, Z);
  p[0] = 1;
  {
    var i = 1;
    while ((i < MaxN))
    {
      p[i] = ((p[(i - 1)] * 10) % Mod);
      i += 1;
    }
  }
  end_state = ((((1 << X)) | ((1 << ((X + Y))))) | ((1 << (((X + Y) + Z)))));
  write(dp(0, 1), "\n");
  return 0;
}
