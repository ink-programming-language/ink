// Translated from solution.cpp.

var LINF: dynamic = 1e18;

var INF: dynamic = 1e9;

var M: dynamic = (1e9 + 7);

var EPS: dynamic = 1.0e-9;

var PI: dynamic = acos(-1.0);

var s: dynamic = cpp_uninitialized();

var ch: dynamic = cpp_array(10010);

var ans: dynamic = cpp_array(10100);

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  read(s);
  var len: dynamic = s.length();
  var pos: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < len))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  return 0;
}
