// Translated from solution.cpp.

var mod = (1e9 + 7);

var mod1 = 998244353;

var inf = 5e18;

var n: dynamic;

var p: dynamic;

var w: dynamic;

var d: dynamic;

func solve()
{
  read(n, p, w, d);
  {
    var draw = 0;
    while ((draw < w))
    {
      var score = (draw * d);
      var win = (((p - score)) / w);
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  solve();
  return 0;
}
