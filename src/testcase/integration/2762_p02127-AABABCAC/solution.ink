// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(ll i=0;i<(ll)(n);i++)");
}

func all(a: dynamic)
{
  return cpp_expression("#include \"bits/stdc++");
}

var pb = cpp_expression("#include \"bi");

var INF = cpp_expression("#includ");

func isSubstr(s: dynamic, t: dynamic)
{
  var p = 0;
  rep(i, s.size());
  {
    if ((s[i] == t[p]))
    {
      p += 1;
    }
    if ((p == t.size()))
    {
      break;
    }
  }
  if ((p == t.size()))
  {
    return true;
  } else
  {
    return false;
  }
}

func main()
{
  var s: dynamic;
  var t: dynamic;
  read(s, t);
  var vs = cpp_construct(2);
  vs[1] = t;
  var ok = 0;
  {
    var i = 1;
    while ((i < 18))
    {
      if (isSubstr(s, vs[(i % 2)]))
      {
        ok = i;
      } else
      {
        break;
      }
      var l = ((((vs[(i % 2)].size() + 1)) * t.size()) + vs[(i % 2)].size());
      if ((l > s.size()))
      {
        break;
      }
      vs[(((i + 1)) % 2)] = t;
      rep(j, vs[(i % 2)].size());
      {
        vs[(((i + 1)) % 2)] += string_cpp(1, vs[(i % 2)][j]);
        vs[(((i + 1)) % 2)] += t;
      }
      i += 1;
    }
  }
  write(ok, "\n");
}
