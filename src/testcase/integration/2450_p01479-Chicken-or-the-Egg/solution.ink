// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(ll i=0;i<(ll)(n);i++)");
}

func all(a: dynamic) -> dynamic
{
  return cpp_expression("#include \"bits/stdc++");
}

var pb: dynamic = cpp_expression("#include");

var INF: dynamic = cpp_expression("#include");

var eps: dynamic = cpp_expression("#inc");

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  read(s);
  var t: dynamic = "";
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      if ((s.substr(i, 3) == "egg"))
      {
        t += cpp_char("1");
        i += 3;
      } else
      {
        t += cpp_char("0");
        i += 7;
      }
    }
  }
  var a: dynamic = cpp_uninitialized();
  var buf: dynamic = "";
  buf += t[0];
  {
    var i: dynamic = 1;
    while ((i < t.size()))
    {
      if ((t[i] != t[(i - 1)]))
      {
        buf += t[i];
      } else
      {
        a.pb(buf);
        buf = "";
        buf += t[i];
      }
      i += 1;
    }
  }
  if (buf.size())
  {
    a.pb(buf);
  }
  var l: dynamic = 0;
  var ans: dynamic = "";
  rep(i, a.size());
  {
    if ((a[i].size() > l))
    {
      ans = a[i];
      l = ans.size();
    }
  }
  if ((ans[(ans.size() - 1)] == cpp_char("0")))
  {
    write("chicken", "\n");
  } else
  {
    write("egg", "\n");
  }
}
