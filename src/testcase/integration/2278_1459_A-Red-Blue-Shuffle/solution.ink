// Translated from solution.cpp.

var F: dynamic = cpp_expression("#incl");

var S: dynamic = cpp_expression("#inclu");

var mp: dynamic = cpp_expression("#include");

var pb: dynamic = cpp_expression("#include");

var popb: dynamic = cpp_expression("#include <");

var ll: dynamic = dynamic;

func sz(x: dynamic) -> dynamic
{
  return cpp_expression("#include <map");
}

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include <map> #in");
}

func forn(x: dynamic) -> dynamic
{
  cpp_macro("for(int i=1;i<=x;i++)");
}

func sforn(x: dynamic, y: dynamic) -> dynamic
{
  cpp_macro("for(int i=1;i<=x;i++)for(int j=1;j<=y;j++)");
}

var pii: dynamic = cpp_expression("#include <map>");

var speed: dynamic = cpp_expression("#include <map> #include <set>");

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var s1: dynamic = cpp_uninitialized();
  var s2: dynamic = cpp_uninitialized();
  read(s1, s2);
  var cnt1: dynamic = 0;
  var cnt2: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < s1.size()))
    {
      if ((s1[i] > s2[i]))
      {
        cnt1 += 1;
      } else if ((s1[i] < s2[i]))
      {
        cnt2 += 1;
      }
      i += 1;
    }
  }
  if ((cnt1 > cnt2))
  {
    write("RED", "\n");
  } else if ((cnt1 < cnt2))
  {
    write("BLUE", "\n");
  } else
  {
    write("EQUAL", "\n");
  }
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  {
    var i: dynamic = 1;
    while ((i <= t))
    {
      solve();
      i += 1;
    }
  }
}
