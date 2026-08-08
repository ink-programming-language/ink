// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n = 0;
    read(n);
    var mp = 0;
    var mc = 0;
    var is = 1;
    while (true)
    {
      var p: dynamic;
      var c: dynamic;
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
