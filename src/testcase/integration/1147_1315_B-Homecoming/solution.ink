// Translated from solution.cpp.

var INF = int_cpp(1e9);

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic;
    var b: dynamic;
    var p: dynamic;
    read(a, b, p);
    var s: dynamic;
    read(s);
    var n = s.size();
    var ans = s.size();
    var cost = 0;
    {
      var i = (n - 2);
      while ((i >= 0))
      {
        var j = i;
        while (((j >= 0) && (s[i] == s[j])))
        {
          j -= 1;
        }
        if ((s[i] == cpp_char("A")))
        {
          cost += cpp_cast(a);
        } else
        {
          cost += cpp_cast(b);
        }
        if ((cost <= p))
        {
          ans = min(ans, (j + 2));
        }
        i = j;
      }
    }
    write(ans, cpp_char("\n"));
  }
  return 0;
}
