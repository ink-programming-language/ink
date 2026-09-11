// Translated from solution.cpp.

var CRT_SECURE_NO_WARNINGS: dynamic = cpp_expression("#def");

func all(c: dynamic) -> dynamic
{
  return cpp_expression("#define _CRT_SECURE_NO");
}

func loop(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(ll i=a; i<ll(b); i++)");
}

func rep(i: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#define _CR");
}

var pb: dynamic = cpp_expression("#define _");

var eb: dynamic = cpp_expression("#define _CRT");

var mp: dynamic = cpp_expression("#define _");

var mt: dynamic = cpp_expression("#define _C");

var lb: dynamic = cpp_expression("#define _CR");

var ub: dynamic = cpp_expression("#define _CR");

func dump() -> dynamic
{
  return cpp_expression("#define _CRT_SECURE_NO_WARNINGS #include <bits/stdc++.h> using namespace std; type");
}

class DUMP
{
  func operator(t: dynamic) -> dynamic
  {
      if (self->tellp())
      {
        ((*self) << ", ");
      }
      ((*self) << t);
      return (*self);
    }
}

func dump() -> dynamic
{
  cpp_macro("");
}

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  ((rep(i, v.size()) << v[i]) << ( (((i + 1) == v.size())) ? "" : " "));
  return os;
}

var T: dynamic = cpp_uninitialized();

var t: dynamic = cpp_array(40);

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_array(200);

var last: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(200);

func rec(x: dynamic) -> dynamic
{
  if ((x >= last))
  {
    return 0;
  }
  var res: dynamic = dp[x];
  if ((res != -1))
  {
    return res;
  }
  res = (1 << 29);
  var ok: dynamic = true;
  {
    var i: dynamic = x;
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
    var i: dynamic = (x + 1);
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

func main() -> dynamic
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      M[x] = min(M[x], y);
      last = max(last, x);
    }
