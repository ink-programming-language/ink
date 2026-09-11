// Translated from solution.cpp.

var INF: dynamic = int_cpp(1e9);

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var p: dynamic = cpp_uninitialized();
    read(a, b, p);
    var s: dynamic = cpp_uninitialized();
    read(s);
    var n: dynamic = s.size();
    var ans: dynamic = s.size();
    var cost: dynamic = 0;
    {
      var i: dynamic = (n - 2);
      while ((i >= 0))
      {
        var j: dynamic = i;
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
