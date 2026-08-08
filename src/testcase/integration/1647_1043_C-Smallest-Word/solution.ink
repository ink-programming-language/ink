// Translated from solution.cpp.

var LINF = 1e18;

var INF = 1e9;

var M = (1e9 + 7);

var EPS = 1.0e-9;

var PI = acos(-1.0);

var s: dynamic;

var ch = cpp_array(10010);

var ans = cpp_array(10100);

func main()
{
  ios_base.sync_with_stdio(0);
  read(s);
  var len = s.length();
  var pos = 0;
  while ((pos < (len - 1)))
  {
    if ((s[pos] != s[(pos + 1)]))
    {
      ans[pos] = 1;
    }
    pos += 1;
  }
  if ((s[(len - 1)] == cpp_char("a")))
  {
    ans[(len - 1)] = 1;
  }
  {
    var i = 0;
    while ((i < len))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  return 0;
}
