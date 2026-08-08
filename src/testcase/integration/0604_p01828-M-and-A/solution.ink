// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0; i<n; ++i)");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a; i<=b; ++i)");
}

func FORR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for (int i=a; i>=b; --i)");
}

var pi = cpp_expression("#inc");

func main()
{
  var s: dynamic;
  var t: dynamic;
  var s1: dynamic;
  var s2: dynamic;
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
  var j1 = 0;
  var j2 = 0;
  {
    var i = 0;
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
