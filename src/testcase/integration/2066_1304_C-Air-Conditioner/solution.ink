// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  while (cpp_update(n, "--"))
  {
    var flag: dynamic = 1;
    var mx: dynamic = cpp_uninitialized();
    var mi: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    var t: dynamic = cpp_uninitialized();
    var time: dynamic = cpp_uninitialized();
    read(m, t);
    mx = cpp_assign(mi, "=", t);
    time = 0;
    while (cpp_update(m, "--"))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      var c: dynamic = cpp_uninitialized();
      read(a, b, c);
      mx = (mx + ((a - time)));
      mi = (mi - ((a - time)));
      time = a;
      if (((mx < b) || (mi > c)))
      {
        flag = 0;
      } else
      {
        mx = min(mx, c);
        mi = max(mi, b);
      }
    }
    if (flag)
    {
      write("YES\n");
    } else
    {
      write("NO\n");
    }
  }
}
