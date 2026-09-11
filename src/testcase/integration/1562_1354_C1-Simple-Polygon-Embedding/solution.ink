// Translated from solution.cpp.

var mod: dynamic = 998244353;

var eps: dynamic = 1e-10;

var INF: dynamic = 0x3f3f3f3f;

var MAXN: dynamic = (2e3 + 10);

var maxn: dynamic = (1e5 + 10);

var inf: dynamic = 100000000000000;

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    var pi: dynamic = (2 * acos(0.0));
    var a: dynamic = (0.5 / sin(((pi / n) / 2.0)));
    var ans: dynamic = (a * cos(((pi / 4.0) / n)));
    ans = (ans * 2);
    write(fixed);
    write(setprecision(8));
    write(ans, "\n");
  }
}
