// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0; i<n; ++i)");
}

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a; i<=b; ++i)");
}

func FORR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for (int i=a; i>=b; --i)");
}

var pi: dynamic = cpp_expression("#inc");

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var s1: dynamic = cpp_uninitialized();
  var s2: dynamic = cpp_uninitialized();
  read(s, t);
  REP(i, s.length());
  {
    if (((i % 2) == 0))
    {
      s1 += s[i];
    } else
    {
      s2 += s[i];
    }
  }
  var j1: dynamic = 0;
  var j2: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((((i < t.length()) && (j1 < s1.length())) && (j2 < s2.length())))
    {
      if ((t[i] == s1[j1]))
      {
        j1 += 1;
      }
      if ((t[i] == s2[j2]))
      {
        j2 += 1;
      }
      i += 1;
    }
  }
  if (((j1 == s1.length()) || (j2 == s2.length())))
  {
    write("Yes", "\n");
  } else
  {
    write("No", "\n");
  }
  return 0;
}
