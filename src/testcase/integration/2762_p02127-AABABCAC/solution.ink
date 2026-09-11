// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(ll i=0;i<(ll)(n);i++)");
}

func all(a: dynamic) -> dynamic
{
  return cpp_expression("#include \"bits/stdc++");
}

var pb: dynamic = cpp_expression("#include \"bi");

var INF: dynamic = cpp_expression("#includ");

func isSubstr(s: dynamic, t: dynamic) -> dynamic
{
  var p: dynamic = 0;
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

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(s, t);
  var vs: dynamic = cpp_construct(2);
  vs[1] = t;
  var ok: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i < 18))
    {
      if (isSubstr(s, vs[(i % 2)]))
      {
        ok = i;
      } else
      {
        break;
      }
      var l: dynamic = ((((vs[(i % 2)].size() + 1)) * t.size()) + vs[(i % 2)].size());
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
