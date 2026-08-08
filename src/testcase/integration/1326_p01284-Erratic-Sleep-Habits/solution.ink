// Translated from solution.cpp.

var CRT_SECURE_NO_WARNINGS = cpp_expression("#def");

func all(c: dynamic)
{
  return cpp_expression("#define _CRT_SECURE_NO");
}

func loop(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(ll i=a; i<ll(b); i++)");
}

func rep(i: dynamic, b: dynamic)
{
  return cpp_expression("#define _CR");
}

var pb = cpp_expression("#define _");

var eb = cpp_expression("#define _CRT");

var mp = cpp_expression("#define _");

var mt = cpp_expression("#define _C");

var lb = cpp_expression("#define _CR");

var ub = cpp_expression("#define _CR");

func dump()
{
  return cpp_expression("#define _CRT_SECURE_NO_WARNINGS #include <bits/stdc++.h> using namespace std; type");
}

class DUMP
{
  func operator(t: dynamic)
  {
      if (this->tellp())
      {
        ((*this) << ", ");
      }
      ((*this) << t);
      return (*this);
    }
}

func dump()
{
  cpp_macro("");
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  ((rep(i, v.size()) << v[i]) << (if (((i + 1) == v.size())) "" else " "));
  return os;
}

var T: dynamic;

var t = cpp_array(40);

var N: dynamic;

var M = cpp_array(200);

var last: dynamic;

var dp = cpp_array(200);

func rec(x: dynamic)
{
  if ((x >= last))
  {
    return 0;
  }
  var res = dp[x];
  if ((res != -1))
  {
    return res;
  }
  res = (1 << 29);
  var ok = true;
  {
    var i = x;
    while ((i <= last))
    {
      if ((M[i] < t[(((i - x)) % T)]))
      {
        ok = false;
      }
      i += 1;
    }
  }
  if (ok)
  {
    res = 0;
  }
  {
    var i = (x + 1);
    while ((i <= last))
    {
      res = min((1 + rec(i)), res);
      if ((M[i] < t[(((i - x)) % T)]))
      {
        break;
      }
      i += 1;
    }
  }
  return res;
}

func main()
{
  while (((cin >> T) && T))
  {
    memset(dp, -1, cpp_sizeof((dp)));
    rep(i, 200)[i] = 24;
    rep(i, T);
    read(t[i]);
    read(N);
    write(rec(1), "\n");
  }
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      M[x] = min(M[x], y);
      last = max(last, x);
    }
