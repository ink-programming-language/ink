// Translated from solution.cpp.

var USE_MATH_DEFINES = cpp_expression("#def");

var CRT_SECURE_NO_WARNINGS = cpp_expression("#def");

var inf = (1 << 60);

var mod = (cpp_cast(1e9) + 7);

func all(v: dynamic)
{
  return cpp_expression("#define _USE_MATH_");
}

func rall(v: dynamic)
{
  return cpp_expression("#define _USE_MATH_DE");
}

func print(s: dynamic)
{
  cpp_macro("cout << s;");
}

func println(s: dynamic)
{
  cpp_macro("cout << s << endl;");
}

func printd(s: dynamic, f: dynamic)
{
  cpp_macro("cout << fixed << setprecision(f) << s << endl;");
}

func main()
{
  var n: dynamic;
  read(n);
  var a: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var q: dynamic;
      read(q);
      var s: dynamic;
      var __cpp_switch_1 = q;
      if (__cpp_switch_1 == 0)
      {
        read(s);
        var x: dynamic;
        read(x);
        a.emplace(s, x);
        break;
      }
      else if (__cpp_switch_1 == 1)
      {
        read(s);
        if ((a.count(s) > 0))
        {
        var p = a.equal_range(s);
        {
        var it = p.first;
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
        var l: dynamic;
        var r: dynamic;
        read(l, r);
        var p = a.equal_range(l);
        var q = a.equal_range(r);
        {
        var it = p.first;
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
