// Translated from solution.cpp.

var a = cpp_array(2345, 2345);

var d = cpp_array(2345, 2345);

var dx = [1, 0, 0, -1];

var dy = [0, 1, -1, 0];

func main()
{
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  read(n, m, k);
  var s = "ULRD";
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= m))
        {
          read(a[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    j = 1;
    while ((j <= m))
    {
      var d = 0;
      {
        i = 1;
        while ((i <= n))
        {
          var t = ((i - 1));
          {
            k = 0;
            while ((k < 4))
            {
              var x = (i + (dx[k] * t));
              var y = (j + (dy[k] * t));
              if ((((((x >= 1) && (x <= n)) && (y <= m)) && (y >= 1)) && (a[x][y] == s[k])))
              {
                d += 1;
              }
              k += 1;
            }
          }
          i += 1;
        }
      }
      write(d, " ");
      j += 1;
    }
  }
}
