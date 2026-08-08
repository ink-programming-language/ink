// Translated from solution.cpp.

var F = cpp_expression("#incl");

var S = cpp_expression("#inclu");

var mp = cpp_expression("#include");

var pb = cpp_expression("#include");

var popb = cpp_expression("#include <");

var ll = dynamic;

func sz(x: dynamic)
{
  return cpp_expression("#include <map");
}

func all(x: dynamic)
{
  return cpp_expression("#include <map> #in");
}

func forn(x: dynamic)
{
  cpp_macro("for(int i=1;i<=x;i++)");
}

func sforn(x: dynamic, y: dynamic)
{
  cpp_macro("for(int i=1;i<=x;i++)for(int j=1;j<=y;j++)");
}

var pii = cpp_expression("#include <map>");

var speed = cpp_expression("#include <map> #include <set>");

func solve()
{
  var n: dynamic;
  read(n);
  var s1: dynamic;
  var s2: dynamic;
  read(s1, s2);
  var cnt1 = 0;
  var cnt2 = 0;
  {
    var i = 0;
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

func main()
{
  var t: dynamic;
  read(t);
  {
    var i = 1;
    while ((i <= t))
    {
      solve();
      i += 1;
    }
  }
}
