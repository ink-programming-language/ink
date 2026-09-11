// Translated from solution.cpp.

var USE_MATH_DEFINES: dynamic = cpp_expression("#def");

var CRT_SECURE_NO_WARNINGS: dynamic = cpp_expression("#def");

var inf: dynamic = (1 << 60);

var mod: dynamic = (cpp_cast(1e9) + 7);

func all(v: dynamic) -> dynamic
{
  return cpp_expression("#define _USE_MATH_");
}

func rall(v: dynamic) -> dynamic
{
  return cpp_expression("#define _USE_MATH_DE");
}

func print(s: dynamic) -> dynamic
{
  cpp_macro("cout << s;");
}

func println(s: dynamic) -> dynamic
{
  cpp_macro("cout << s << endl;");
}

func printd(s: dynamic, f: dynamic) -> dynamic
{
  cpp_macro("cout << fixed << setprecision(f) << s << endl;");
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var q: dynamic = cpp_uninitialized();
      read(q);
      var s: dynamic = cpp_uninitialized();
      var __cpp_switch_1: dynamic = q;
      if (__cpp_switch_1 == 0)
      {
        read(s);
        var x: dynamic = cpp_uninitialized();
        read(x);
        a.emplace(s, x);
        break;
      }
      else if (__cpp_switch_1 == 1)
      {
        read(s);
        if ((a.count(s) > 0))
        {
        var p: dynamic = a.equal_range(s);
        {
        var it: dynamic = p.first;
        while ((it != p.second))
        {
        println(it->second);
        it += 1;
        }
        }
        } else
        {
        }
        break;
      }
      else if (__cpp_switch_1 == 2)
      {
        read(s);
        if (a.count(s))
        {
        a.erase(s);
        }
        break;
      }
      else if (__cpp_switch_1 == 3)
      {
        var l: dynamic = cpp_uninitialized();
        var r: dynamic = cpp_uninitialized();
        read(l, r);
        var p: dynamic = a.equal_range(l);
        var q: dynamic = a.equal_range(r);
        {
        var it: dynamic = p.first;
        while ((it != q.second))
        {
        println(((it->first << " ") << it->second));
        it += 1;
        }
        }
        break;
      }
      i += 1;
    }
  }
}
