// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

var mod1: dynamic = 998244353;

var inf: dynamic = 5e18;

var n: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

func solve() -> dynamic
{
  read(n, p, w, d);
  {
    var draw: dynamic = 0;
    while ((draw < w))
    {
      var score: dynamic = (draw * d);
      var win: dynamic = (((p - score)) / w);
      if (((((win >= 0) && (score >= 0)) && ((score + (win * w)) == p)) && ((win + draw) <= n)))
      {
        write(win, " ", draw, " ", ((n - win) - draw));
        return;
      }
      draw += 1;
    }
  }
  write(-1);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  solve();
  return 0;
}
