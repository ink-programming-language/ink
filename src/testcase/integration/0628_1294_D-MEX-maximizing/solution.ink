// Translated from solution.cpp.

var N: dynamic = (1e5 + 5);

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var mini: dynamic = cpp_uninitialized();
  var maxi: dynamic = INT_MAX;
  var prev: dynamic = cpp_uninitialized();
  read(q, x);
  var m1: dynamic = cpp_uninitialized();
  var m2: dynamic = cpp_uninitialized();
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

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
