// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var m: dynamic;
    var a: dynamic;
    var b: dynamic;
    read(n, m, a, b);
    if ((((n * a)) != ((m * b))))
    {
      write("NO", "\n");
    } else
    {
      write("YES", "\n");
      var A = cpp_array(m, n);
      memset(A, 0, cpp_sizeof((A)));
      var x = 0;
      {
        var i = 0;
        while ((i < n))
        {
          var c = a;
          while (cpp_update(c, "--"))
          {
            A[i][x] = 1;
            x += 1;
            x = (x % m);
          }
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
            while ((j < m))
            {
              write(A[i][j]);
              j += 1;
            }
          }
          write("\n");
          i += 1;
        }
      }
    }
  }
}
