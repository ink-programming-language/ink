// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int (i)=0;(i)<(int)(n);++(i))");
}

func all(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

var pb = cpp_expression("#include");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func dbg(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> u");
}

func operator_shift_left(o: dynamic, p: dynamic)
{
  (((((o << "(") << p.fi) << ",") << p.se) << ")");
  return o;
}

func operator_shift_left(o: dynamic, v: dynamic)
{
  (o << "[");
  for (var t in v)
  {
    ((o << t) << ",");
  }
  (o << "]");
  return o;
}

func PRINT(a: dynamic)
{
  for (var p in a)
  {
    write(p.se, p.fi, " + ");
  }
  write("\n");
}

func norm(a: dynamic)
{
  var ret: dynamic;
  for (var p in a)
  {
    var v = p.fi;
    sort(all(v));
    if ((p.se != 0))
    {
      ret[v] = p.se;
    }
  }
  return ret;
}

func sub(a: dynamic)
{
  a = norm(a);
  var ret: dynamic;
  for (var p in a)
  {
    var v = p.fi;
    if ((p.se != 0))
    {
      ret[v] = (-p.se);
    }
  }
  return ret;
}

func add(a: dynamic, b: dynamic)
{
  a = norm(a);
  b = norm(b);
  for (var p in b)
  {
    a[p.fi] += p.se;
  }
  return norm(a);
}

func mul(a: dynamic, b: dynamic)
{
  a = norm(a);
  b = norm(b);
  var ret: dynamic;
  for (var p in a)
  {
    for (var q in b)
    {
      var var_cpp = (p.fi + q.fi);
      sort(all(var_cpp));
      ret[var_cpp] += (p.se * q.se);
    }
  }
  return norm(ret);
}

func T(s: dynamic)
{
  var n = s.size();
  var ret: dynamic;
  ret[""] = 1;
  var idx = 0;
  while ((idx < n))
  {
    var m: dynamic;
    if ((s[idx] == cpp_char(" ")))
    {
      idx += 1;
      continue;
    } else if ((s[idx] == cpp_char("(")))
    {
      var ep = idx;
      var p = 0;
      while ((ep < n))
      {
        if ((s[ep] == cpp_char("(")))
        {
          p += 1;
        }
        if ((s[ep] == cpp_char(")")))
        {
          p -= 1;
          if ((p == 0))
          {
            break;
          }
        }
        ep += 1;
      }
      assert((ep < n));
      assert((s[ep] == cpp_char(")")));
      var t = s.substr((idx + 1), ((ep - idx) - 1));
      m = E(t);
      idx = (ep + 1);
    } else if (isdigit(s[idx]))
    {
      var val = 0;
      while (((idx < n) && isdigit(s[idx])))
      {
        val = ((val * 10) + ((s[idx] - cpp_char("0"))));
        idx += 1;
      }
      var pw = 1;
      var nx = idx;
      while (((nx < n) && (s[nx] == cpp_char(" "))))
      {
        nx += 1;
      }
      if (((nx < n) && (s[nx] == cpp_char("^"))))
      {
        nx += 1;
        while (((nx < n) && (s[nx] == cpp_char(" "))))
        {
          nx += 1;
        }
        assert((nx < n));
        assert(isdigit(s[nx]));
        pw = 0;
        while (((nx < n) && isdigit(s[nx])))
        {
          pw = ((10 * pw) + ((s[nx] - cpp_char("0"))));
          nx += 1;
        }
      }
      var vv = 1;
      rep(cpp_name, pw) *= val;
      m[""] = vv;
      idx = nx;
    } else if (islower(s[idx]))
    {
      var c = s[idx];
      idx += 1;
      var pw = 1;
      var nx = idx;
      while (((nx < n) && (s[nx] == cpp_char(" "))))
      {
        nx += 1;
      }
      if (((nx < n) && (s[nx] == cpp_char("^"))))
      {
        nx += 1;
        while (((nx < n) && (s[nx] == cpp_char(" "))))
        {
          nx += 1;
        }
        assert((nx < n));
        assert(isdigit(s[nx]));
        pw = 0;
        while (((nx < n) && isdigit(s[nx])))
        {
          pw = ((10 * pw) + ((s[nx] - cpp_char("0"))));
          nx += 1;
        }
      }
      var t = "";
      rep(cpp_name, pw) += c;
      m[t] = 1;
      idx = nx;
    } else
    {
      assert(false);
    }
    ret = mul(ret, m);
  }
  return norm(ret);
}

func E(s: dynamic)
{
  var n = s.size();
  var ret: dynamic;
  var p = 0;
  var start = 0;
  var plus = true;
  var term = s.substr(start, (n - start));
  var t = norm(T(term));
  if ((!plus))
  {
    t = sub(t);
  }
  ret = add(ret, t);
  return norm(ret);
}

func main()
{
  var s: dynamic;
  while (cpp_comma(getline(cin, s), ((s != "."))))
  {
    var S = E(s);
    var t: dynamic;
    while (cpp_comma(getline(cin, t), ((t != "."))))
    {
      var T = E(t);
      write((if ((S == T)) "yes" else "no"), "\n");
    }
    write(".", "\n");
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if ((s[i] == cpp_char("(")))
    {
      p += 1;
    }
    if ((s[i] == cpp_char(")")))
    {
      p -= 1;
    }
    if (((s[i] == cpp_char("+")) || (s[i] == cpp_char("-"))))
    {
      if ((p == 0))
      {
        var term = s.substr(start, (i - start));
        var t = norm(T(term));
        if ((!plus))
        {
          t = sub(t);
        }
        plus = ((s[i] == cpp_char("+")));
        ret = add(ret, t);
        start = (i + 1);
      }
    }
  }
