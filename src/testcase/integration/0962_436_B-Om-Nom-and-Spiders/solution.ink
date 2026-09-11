// Translated from solution.cpp.

var a: dynamic = cpp_array(2345, 2345);

var d: dynamic = cpp_array(2345, 2345);

var dx: dynamic = [1, 0, 0, -1];

var dy: dynamic = [0, 1, -1, 0];

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, m, k);
  var s: dynamic = "ULRD";
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
      var d: dynamic = 0;
      {
        i = 1;
        while ((i <= n))
        {
          var t: dynamic = ((i - 1));
          {
            k = 0;
            while ((k < 4))
            {
              var x: dynamic = (i + (dx[k] * t));
              var y: dynamic = (j + (dy[k] * t));
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
