// Translated from solution.cpp.

var ss = [[0, 1, 1, 1, 1, 1, 1], [0, 0, 0, 0, 1, 1, 0], [1, 0, 1, 1, 0, 1, 1], [1, 0, 0, 1, 1, 1, 1], [1, 1, 0, 0, 1, 1, 0], [1, 1, 0, 1, 1, 0, 1], [1, 1, 1, 1, 1, 0, 1], [0, 1, 0, 0, 1, 1, 1], [1, 1, 1, 1, 1, 1, 1], [1, 1, 0, 1, 1, 1, 1]];

func main()
{
  {
    while (true)
    {
      var n: dynamic;
      read(n);
      if ((n == -1))
      {
        return 0;
      }
      var d = cpp_array(7);
      fill(d, (d + 7), 0);
      {
        var i = 0;
        while ((i < n))
        {
          var x: dynamic;
          read(x);
          {
            var j = 0;
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
