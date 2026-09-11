// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    read(n, m, a, b);
    if ((((n * a)) != ((m * b))))
    {
      write("NO", "\n");
    } else
    {
      write("YES", "\n");
      var A: dynamic = cpp_array(m, n);
      memset(A, 0, cpp_sizeof((A)));
      var x: dynamic = 0;
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          var c: dynamic = a;
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
        var i: dynamic = 0;
        while ((i < n))
        {
          {
            var j: dynamic = 0;
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
