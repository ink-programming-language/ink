// Translated from solution.cpp.

func main() -> dynamic
{
  {
    var L: dynamic = cpp_uninitialized();
    while (cpp_comma((cin >> L), L))
    {
      var m: dynamic = 0;
      {
        var i: dynamic = 1;
        while ((i <= 12))
        {
          var M: dynamic = cpp_uninitialized();
          var N: dynamic = cpp_uninitialized();
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
