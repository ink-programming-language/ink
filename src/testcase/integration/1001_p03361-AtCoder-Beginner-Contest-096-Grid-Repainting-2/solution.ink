// Translated from solution.cpp.

var h: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var fl: dynamic = 1;

var a: dynamic = cpp_array(55, 55);

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  read(h, w);
  {
    i = 1;
    while ((i <= h))
    {
      read(s);
      {
        j = 1;
        while ((j <= w))
        {
          a[i][j] =  ((s[(j - 1)] == cpp_char("."))) ? 0 : 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= h))
    {
      {
        j = 1;
        while ((j <= w))
        {
          if (((a[i][j] == 1) && ((((a[(i - 1)][j] + a[(i + 1)][j]) + a[i][(j - 1)]) + a[i][(j + 1)]) == 0)))
          {
            fl = 0;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(( (fl) ? "Yes" : "No"));
  return 0;
}
