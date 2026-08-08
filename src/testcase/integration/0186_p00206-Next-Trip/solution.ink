// Translated from solution.cpp.

func main()
{
  {
    var L: dynamic;
    while (cpp_comma((cin >> L), L))
    {
      var m = 0;
      {
        var i = 1;
        while ((i <= 12))
        {
          var M: dynamic;
          var N: dynamic;
          read(M, N);
          L -= (M - N);
          if (((m == 0) && (L <= 0)))
          {
            m = i;
          }
          i += 1;
        }
      }
      if (m)
      {
        write(m, "\n");
      } else
      {
        write("NA", "\n");
      }
    }
  }
  return 0;
}
