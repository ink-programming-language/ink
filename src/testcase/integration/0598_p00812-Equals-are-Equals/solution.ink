// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int (i)=0;(i)<(int)(n);++(i))");
}

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

var pb: dynamic = cpp_expression("#include");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

func dbg(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> u");
}

func operator_shift_left(o: dynamic, p: dynamic) -> dynamic
{
  (((((o << "(") << p.fi) << ",") << p.se) << ")");
  return o;
}

func operator_shift_left(o: dynamic, v: dynamic) -> dynamic
{
  (o << "[");
  for (var t: dynamic in v)
  {
    ((o << t) << ",");
  }
  (o << "]");
  return o;
}

func PRINT(a: dynamic) -> dynamic
{
  for (var p: dynamic in a)
  {
    write(p.se, p.fi, " + ");
  }
  write("\n");
}

func norm(a: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  for (var p: dynamic in a)
  {
    var v: dynamic = p.fi;
    sort(all(v));
    if ((p.se != 0))
    {
      ret[v] = p.se;
    }
  }
  return ret;
}

func sub(a: dynamic) -> dynamic
{
  a = norm(a);
  var ret: dynamic = cpp_uninitialized();
  for (var p: dynamic in a)
  {
    var v: dynamic = p.fi;
    if ((p.se != 0))
    {
      ret[v] = (-p.se);
    }
  }
  return ret;
}

func add(a: dynamic, b: dynamic) -> dynamic
{
  a = norm(a);
  b = norm(b);
  for (var p: dynamic in b)
  {
    a[p.fi] += p.se;
  }
  return norm(a);
}

func mul(a: dynamic, b: dynamic) -> dynamic
{
  a = norm(a);
  b = norm(b);
  var ret: dynamic = cpp_uninitialized();
  for (var p: dynamic in a)
  {
    for (var q: dynamic in b)
    {
      var var_cpp: dynamic = (p.fi + q.fi);
      sort(all(var_cpp));
      ret[var_cpp] += (p.se * q.se);
    }
  }
  return norm(ret);
}

func T(s: dynamic) -> dynamic
{
  var n: dynamic = s.size();
  var ret: dynamic = cpp_uninitialized();
  ret[""] = 1;
  var idx: dynamic = 0;
  while ((idx < n))
  {
    var m: dynamic = cpp_uninitialized();
    if ((s[idx] == cpp_char(" ")))
    {
      idx += 1;
      continue;
    } else if ((s[idx] == cpp_char("(")))
    {
      var ep: dynamic = idx;
      var p: dynamic = 0;
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
      var t: dynamic = s.substr((idx + 1), ((ep - idx) - 1));
      m = E(t);
      idx = (ep + 1);
    } else if (isdigit(s[idx]))
    {
      var val: dynamic = 0;
      while (((idx < n) && isdigit(s[idx])))
      {
        val = ((val * 10) + ((s[idx] - cpp_char("0"))));
        idx += 1;
      }
      var pw: dynamic = 1;
      var nx: dynamic = idx;
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
      var vv: dynamic = 1;
      rep(cpp_name, pw) *= val;
      m[""] = vv;
      idx = nx;
    } else if (islower(s[idx]))
    {
      var c: dynamic = s[idx];
      idx += 1;
      var pw: dynamic = 1;
      var nx: dynamic = idx;
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
      var t: dynamic = "";
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

func E(s: dynamic) -> dynamic
{
  var n: dynamic = s.size();
  var ret: dynamic = cpp_uninitialized();
  var p: dynamic = 0;
  var start: dynamic = 0;
  var plus: dynamic = true;
  var term: dynamic = s.substr(start, (n - start));
  var t: dynamic = norm(T(term));
  if ((!plus))
  {
    t = sub(t);
  }
  ret = add(ret, t);
  return norm(ret);
}

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  while (cpp_comma(getline(cin, s), ((s != "."))))
  {
    var S: dynamic = E(s);
    var t: dynamic = cpp_uninitialized();
    while (cpp_comma(getline(cin, t), ((t != "."))))
    {
      var T: dynamic = E(t);
      write(( ((S == T)) ? "yes" : "no"), "\n");
    }
    write(".", "\n");
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
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
        var term: dynamic = s.substr(start, (i - start));
        var t: dynamic = norm(T(term));
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
