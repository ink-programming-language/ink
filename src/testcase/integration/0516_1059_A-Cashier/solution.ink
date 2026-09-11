// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var L: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  read(n, L, a);
  var ans: dynamic = 0;
  var prevEnd: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var t: dynamic = cpp_uninitialized();
      var l: dynamic = cpp_uninitialized();
      read(t, l);
      ans += (((t - prevEnd)) / a);
      prevEnd = (t + l);
      i += 1;
    }
  }
  ans += (((L - prevEnd)) / a);
  write(ans, "\n");
  return 0;
}
