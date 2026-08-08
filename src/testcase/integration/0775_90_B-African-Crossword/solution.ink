// Translated from solution.cpp.

func main()
{
  var a = cpp_array(150, 150);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var s = cpp_array(150, 150);
  memset(s, 0, cpp_sizeof((s)));
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          var c = a[i][j];
          {
            var b = 0;
            while ((b < m))
            {
              if ((b != j))
              {
                if ((c == a[i][b]))
                {
                  s[i][b] = 1;
                }
              }
              b += 1;
            }
          }
          {
            var b = 0;
            while ((b < n))
            {
              if ((b != i))
              {
                if ((c == a[b][j]))
                {
                  s[b][j] = 1;
                }
              }
              b += 1;
            }
          }
          j += 1;
        }
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
          if ((s[i][j] != 1))
          {
            write(a[i][j]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write("\n");
  return 0;
}
