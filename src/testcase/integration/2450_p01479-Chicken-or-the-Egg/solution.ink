// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(ll i=0;i<(ll)(n);i++)");
}

func all(a: dynamic)
{
  return cpp_expression("#include \"bits/stdc++");
}

var pb = cpp_expression("#include");

var INF = cpp_expression("#include");

var eps = cpp_expression("#inc");

func main()
{
  var s: dynamic;
  read(s);
  var t = "";
  {
    var i = 0;
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
  var a: dynamic;
  var buf = "";
  buf += t[0];
  {
    var i = 1;
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
  var l = 0;
  var ans = "";
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
