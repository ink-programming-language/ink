// Translated from solution.cpp.

var mod = 998244353;

var eps = 1e-10;

var INF = 0x3f3f3f3f;

var MAXN = (2e3 + 10);

var maxn = (1e5 + 10);

var inf = 100000000000000;

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    var pi = (2 * acos(0.0));
    var a = (0.5 / sin(((pi / n) / 2.0)));
    var ans = (a * cos(((pi / 4.0) / n)));
    ans = (ans * 2);
    write(fixed);
    write(setprecision(8));
    write(ans, "\n");
  }
}
