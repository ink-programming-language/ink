// Translated from solution.cpp.

var h: dynamic;

var w: dynamic;

var fl = 1;

var a = cpp_array(55, 55);

var s: dynamic;

func main()
{
  var i: dynamic;
  var j: dynamic;
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
          a[i][j] = if ((s[(j - 1)] == cpp_char("."))) 0 else 1;
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
  write((if (fl) "Yes" else "No"));
  return 0;
}
