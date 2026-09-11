// Translated from solution.cpp.

var ss: dynamic = [[0, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 1, 1, 0], [1, 0, 1, 1, 0, 1, 1], [1, 0, 0, 1, 1, 1, 1], [1, 1, 0, 0, 1, 1, 0], [1, 1, 0, 1, 1, 0, 1], [1, 1, 1, 1, 1, 0, 1], [0, 1, 0, 0, 1, 1, 1], [1, 1, 1, 1, 1, 1, 1], [1, 1, 0, 1, 1, 1, 1]];

func main() -> dynamic
{
  {
    while (true)
    {
      var n: dynamic = cpp_uninitialized();
      read(n);
      if ((n == -1))
      {
        return 0;
      }
      var d: dynamic = cpp_array(7);
      fill(d, (d + 7), 0);
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          var x: dynamic = cpp_uninitialized();
          read(x);
          {
            var j: dynamic = 0;
            while ((j < 7))
            {
              write(((d[j] ^ ss[x][j])));
              d[j] = ss[x][j];
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
