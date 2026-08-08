// Translated from solution.cpp.

var maxn = (1e5 + 1);

var mod = (1e9 + 7);

var inf = 1e18;

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  read(t);
  {
    var i = 1;
    while ((i <= t))
    {
      read(a, b, c);
      var tmp = min(b, (c / 2));
      b -= tmp;
      tmp += min(a, (b / 2));
      write((tmp * 3), "\n");
      i += 1;
    }
  }
  return 0;
}
