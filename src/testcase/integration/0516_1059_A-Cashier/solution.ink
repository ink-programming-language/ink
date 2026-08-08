// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  var L: dynamic;
  var a: dynamic;
  read(n, L, a);
  var ans = 0;
  var prevEnd = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var t: dynamic;
      var l: dynamic;
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
