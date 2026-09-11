// Translated from solution.cpp.

var maxn: dynamic = (1e5 + 1);

var mod: dynamic = (1e9 + 7);

var inf: dynamic = 1e18;

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  read(t);
  {
    var i: dynamic = 1;
    while ((i <= t))
    {
      read(a, b, c);
      var tmp: dynamic = min(b, (c / 2));
      b -= tmp;
      tmp += min(a, (b / 2));
      write((tmp * 3), "\n");
      i += 1;
    }
  }
  return 0;
}
