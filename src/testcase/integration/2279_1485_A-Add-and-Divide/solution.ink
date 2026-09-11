// Translated from solution.cpp.

var pb: dynamic = cpp_expression("#include");

var mp: dynamic = cpp_expression("#include");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/std");
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    read(a, b);
    var cur: dynamic = 0;
    if ((b == 1))
    {
      b += 1;
      cur += 1;
    }
    var mn: dynamic = INT_MAX;
    while (true)
    {
      var tmp: dynamic = a;
      var res: dynamic = cur;
      while (a)
      {
        a /= b;
        res += 1;
      }
      if ((res <= mn))
      {
        mn = res;
      } else
      {
        break;
      }
      a = tmp;
      cur += 1;
      b += 1;
    }
    write(mn, cpp_char("\n"));
  }
  return 0;
}
