// Translated from solution.cpp.

func main()
{
  var d: dynamic;
  while (cpp_comma((cin >> d), (d >= 0)))
  {
    var a = 0;
    var b = "";
    a = d;
    d -= a;
    {
      var i = 0;
      while ((i < 4))
      {
        b += cpp_cast(((((d * 2)) + cpp_char("0"))));
        d *= 2;
        if ((d >= 1))
        {
          d -= 1;
        }
        i += 1;
      }
    }
    if (((d > 1e-9) || (a >= ((1 << 8)))))
    {
      write("NA", "\n");
    } else
    {
      {
        var i = 0;
        while ((i < 8))
        {
          write((!(!((a & ((1 << ((7 - i)))))))));
          i += 1;
        }
      }
      write(".", b);
      write("\n");
    }
  }
}
