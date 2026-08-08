// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var r = [0];
  var d = [0];
  var r1 = [0];
  var d1 = [0];
  if ((n == 1))
  {
    write(1, "\n", 0, " ", 1, "\n", 0, "\n", "1", "\n");
  } else
  {
    r[0] = -1;
    d[0] = 0;
    d[1] = 1;
    {
      var i = 1;
      while ((i <= (n - 1)))
      {
        d1[0] = r[0];
        {
          var j = 0;
          while ((j <= i))
          {
            r1[j] = d[j];
            j += 1;
          }
        }
        {
          var j = 0;
          while ((j <= i))
          {
            d1[(j + 1)] = (((d[j] + r[(j + 1)])) % 2);
            j += 1;
          }
        }
        d[0] = d1[0];
        {
          var j = 0;
          while ((j <= i))
          {
            r[j] = r1[j];
            j += 1;
          }
        }
        {
          var j = 0;
          while ((j <= i))
          {
            d[(j + 1)] = d1[(j + 1)];
            j += 1;
          }
        }
        i += 1;
      }
    }
    write(n, "\n");
    {
      var j = 0;
      while ((j <= n))
      {
        write(d[j], " ");
        j += 1;
      }
    }
    write("\n", (n - 1), "\n");
    {
      var j = 0;
      while ((j <= (n - 1)))
      {
        write(r[j], " ");
        j += 1;
      }
    }
  }
  return 0;
}
