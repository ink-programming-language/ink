// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var mon = 0;
  var k = cpp_array(10, 10);
  read(a, b);
  {
    var i = 0;
    while ((i <= 9))
    {
      {
        var j = 0;
        while ((j <= 9))
        {
          read(k[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= 9))
    {
      {
        var j = 0;
        while ((j <= 9))
        {
          {
            var kk = 0;
            while ((kk <= 9))
            {
              if ((k[j][kk] > (k[j][i] + k[i][kk])))
              {
                k[j][kk] = (k[j][i] + k[i][kk]);
              }
              kk += 1;
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
    while ((i < a))
    {
      {
        var j = 0;
        while ((j < b))
        {
          read(c);
          if ((c != -1))
          {
            mon += k[c][1];
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(mon, "\n");
}
