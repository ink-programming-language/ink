// Translated from solution.cpp.

var N = (1e5 + 5);

func solve()
{
  var n: dynamic;
  var q: dynamic;
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  var w: dynamic;
  var mini: dynamic;
  var maxi = INT_MAX;
  var prev: dynamic;
  read(q, x);
  var m1: dynamic;
  var m2: dynamic;
  y = 0;
  while (cpp_update(q, "--"))
  {
    read(n);
    n %= x;
    m1[((x * m2[n]) + n)] += 1;
    m2[n] += 1;
    while (m1[y])
    {
      y += 1;
    }
    write(y, "\n");
  }
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
