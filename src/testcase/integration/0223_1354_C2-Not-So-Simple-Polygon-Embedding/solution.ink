// Translated from solution.cpp.

var I_INF: dynamic = numeric_limits.max();

var L_INF: dynamic = numeric_limits.max();

var PI: dynamic = 3.1415926535897932384626433832795028841971;

func solve() -> dynamic
{
  var Q: dynamic = cpp_uninitialized();
  read(Q);
  write(setprecision(20));
  while (cpp_update(Q, "--"))
  {
    var N: dynamic = cpp_uninitialized();
    read(N);
    var phi: dynamic = 0;
    var cnt: dynamic = 0;
    while ((phi < (PI / 4.0)))
    {
      phi += (PI / N);
      cnt += 1;
    }
    var z: dynamic = ((PI / ((2.0 * N))) + ((PI / N) * (((N / 2) - cnt))));
    var th: dynamic = ((PI / 4.0) - z);
    var ans: dynamic = (cos(th) / sin((PI / ((2.0 * N)))));
    write(ans, "\n");
  }
}

func main() -> dynamic
{
  cin.tie(0);
  cout.tie(0);
  ios.sync_with_stdio(false);
  solve();
  write();
  return 0;
}
