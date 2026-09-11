// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = 0;
    read(n);
    var mp: dynamic = 0;
    var mc: dynamic = 0;
    var is: dynamic = 1;
    while (true)
    {
      var p: dynamic = cpp_uninitialized();
      var c: dynamic = cpp_uninitialized();
      read(p, c);
      if (((((((p < mp) || (c < mc)) || (c > p)) || ((p - mp) < (c - mc)))) && is))
      {
        write("NO", "\n");
        is = 0;
      }
      mp = p;
      mc = c;
      if (!((cpp_update(n, "--"))))
      {
        break;
      }
    }
    if (is)
    {
      write("YES", "\n");
    }
  }
  return 0;
}
