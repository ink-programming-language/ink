// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic;
    var b: dynamic;
    var c: dynamic;
    read(a, b, c);
    var v = 0;
    {
      var i = 0;
      while ((i < a.size()))
      {
        if (((a[i] != c[i]) && (b[i] != c[i])))
        {
          v = 1;
          break;
        }
        i += 1;
      }
    }
    if ((v == 1))
    {
      write("NO", "\n");
    } else
    {
      write("YES", "\n");
    }
  }
}
