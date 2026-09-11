// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_array(150, 150);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var s: dynamic = cpp_array(150, 150);
  memset(s, 0, cpp_sizeof((s)));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          var c: dynamic = a[i][j];
          {
            var b: dynamic = 0;
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
            var b: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
