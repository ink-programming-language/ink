// Translated from solution.cpp.

var Mod: dynamic = (1e9 + 7);

var MaxN: dynamic = 41;

var StateLen: dynamic = ((5 + 7) + 5);

var N: dynamic = cpp_uninitialized();

var X: dynamic = cpp_uninitialized();

var Y: dynamic = cpp_uninitialized();

var Z: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(MaxN);

var f: dynamic = cpp_array((1 << StateLen), MaxN);

var end_state: dynamic = cpp_uninitialized();

func dp(i: dynamic, j: dynamic) -> dynamic
{
  if ((i == N))
  {
    return 0;
  }
  if ((~f[i][j]))
  {
    return f[i][j];
  }
  var res: dynamic = 0;
  {
    var k: dynamic = 1;
    while ((k <= 10))
    {
      var next_state: dynamic = (((j << k)) | 1);
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  memset(f, 0xff, cpp_sizeof(f));
  cin.tie(0);
  read(N, X, Y, Z);
  p[0] = 1;
  {
    var i: dynamic = 1;
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
