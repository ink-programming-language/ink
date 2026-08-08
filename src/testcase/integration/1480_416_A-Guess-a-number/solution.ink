// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(0);
  var a: dynamic;
  read(a);
  var l = -2e9;
  var r = 2e9;
  while (cpp_update(a, "--"))
  {
    var c: dynamic;
    var n: dynamic;
    var s: dynamic;
    read(c, n, s);
    if ((c == ">"))
    {
      if ((s == cpp_char("Y")))
      {
        l = max(l, (n + 1));
      } else
      {
        r = min(r, n);
      }
    } else if ((c == "<"))
    {
      if ((s == cpp_char("Y")))
      {
        r = min(r, (n - 1));
      } else
      {
        l = max(l, n);
      }
    } else if ((c == "<="))
    {
      if ((s == cpp_char("Y")))
      {
        r = min(r, n);
      } else
      {
        l = max(l, (n + 1));
      }
    } else
    {
      if ((s == cpp_char("Y")))
      {
        l = max(l, n);
      } else
      {
        r = min(r, (n - 1));
      }
    }
  }
  if ((l <= r))
  {
    write(l);
  } else
  {
    write("Impossible");
  }
}
