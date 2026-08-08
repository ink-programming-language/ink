// Translated from solution.cpp.

var f = cpp_array(105, 3);

var a = cpp_array(105, 3);

var n: dynamic;

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= 2))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          scanf("%d", (&a[i][j]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  f[1][1] = a[1][1];
  {
    var i = 1;
    while ((i <= 2))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          f[i][j] = (max(f[(i - 1)][j], f[i][(j - 1)]) + a[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d", f[2][n]);
}
