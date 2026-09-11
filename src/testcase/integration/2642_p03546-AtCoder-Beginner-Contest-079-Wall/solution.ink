// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var mon: dynamic = 0;
  var k: dynamic = cpp_array(10, 10);
  read(a, b);
  {
    var i: dynamic = 0;
    while ((i <= 9))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i <= 9))
    {
      {
        var j: dynamic = 0;
        while ((j <= 9))
        {
          {
            var kk: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < a))
    {
      {
        var j: dynamic = 0;
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
