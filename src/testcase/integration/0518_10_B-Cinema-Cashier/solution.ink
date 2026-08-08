// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var a = cpp_array(100, 100);

var xc: dynamic;

var yc: dynamic;

func lolol(x: dynamic, y: dynamic)
{
  return (abs((x - xc)) + abs((y - yc)));
}

func main()
{
  read(n, k);
  xc = (((k + 1)) / 2);
  yc = (((k + 1)) / 2);
  {
    var asdasd = 0;
    while ((asdasd < n))
    {
      var m: dynamic;
      read(m);
      var ans = 999999999;
      var xx1: dynamic;
      var yy1: dynamic;
      var p: dynamic;
      {
        var i = 1;
        while ((i <= k))
        {
          p = 0;
          var kol = 0;
          {
            var j = 1;
            while ((j <= k))
            {
              if ((j > m))
              {
                kol -= a[i][(j - m)];
                p -= lolol(i, (j - m));
              }
              kol += a[i][j];
              p += lolol(i, j);
              if ((((j >= m) && (kol == 0)) && (p < ans)))
              {
                ans = p;
                xx1 = i;
                yy1 = j;
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      if ((ans == 999999999))
      {
        write(-1, "\n");
      } else
      {
        write(xx1, " ", ((yy1 - m) + 1), " ", yy1, "\n");
        {
          var j = ((yy1 - m) + 1);
          while ((j <= yy1))
          {
            a[xx1][j] = 1;
            j += 1;
          }
        }
      }
      asdasd += 1;
    }
  }
  return 0;
}
