// Translated from solution.cpp.

var pb = cpp_expression("#include");

var mp = cpp_expression("#include");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func all(x: dynamic)
{
  return cpp_expression("#include <bits/std");
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic;
    var b: dynamic;
    read(a, b);
    var cur = 0;
    if ((b == 1))
    {
      b += 1;
      cur += 1;
    }
    var mn = INT_MAX;
    while (true)
    {
      var tmp = a;
      var res = cur;
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
