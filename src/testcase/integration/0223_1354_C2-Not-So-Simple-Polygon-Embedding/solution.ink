// Translated from solution.cpp.

var I_INF = numeric_limits.max();

var L_INF = numeric_limits.max();

var PI = 3.1415926535897932384626433832795028841971;

func solve()
{
  var Q: dynamic;
  read(Q);
  write(setprecision(20));
  while (cpp_update(Q, "--"))
  {
    var N: dynamic;
    read(N);
    var phi = 0;
    var cnt = 0;
    while ((phi < (PI / 4.0)))
    {
      phi += (PI / N);
      cnt += 1;
    }
    var z = ((PI / ((2.0 * N))) + ((PI / N) * (((N / 2) - cnt))));
    var th = ((PI / 4.0) - z);
    var ans = (cos(th) / sin((PI / ((2.0 * N)))));
    write(ans, "\n");
  }
}

func main()
{
  cin.tie(0);
  cout.tie(0);
  ios.sync_with_stdio(false);
  solve();
  write();
  return 0;
}
