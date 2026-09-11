// Translated from solution.cpp.

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  read(n);
  var a: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x);
      a.push_back(x);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      if ((a[i] != 0))
      {
        ans += a[i];
      }
      var j: dynamic = 1;
      while (((i + j) < n))
      {
        j = ((j << 1));
      }
      j = ((j >> 1));
      a[(i + j)] += a[i];
      write(ans, "\n");
      i += 1;
    }
  }
  return;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic = 1;
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var e: dynamic = cpp_uninitialized();
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
