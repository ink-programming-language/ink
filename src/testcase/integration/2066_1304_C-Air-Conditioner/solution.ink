// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  while (cpp_update(n, "--"))
  {
    var flag = 1;
    var mx: dynamic;
    var mi: dynamic;
    var m: dynamic;
    var t: dynamic;
    var time: dynamic;
    read(m, t);
    mx = cpp_assign(mi, "=", t);
    time = 0;
    while (cpp_update(m, "--"))
    {
      var a: dynamic;
      var b: dynamic;
      var c: dynamic;
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
