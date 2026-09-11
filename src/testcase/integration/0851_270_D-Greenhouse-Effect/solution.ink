// Translated from solution.cpp.

var xx: dynamic = cpp_array(5002, 5002);

func lon_com_sub(ar: dynamic, br: dynamic, n: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i <= n))
    {
      {
        j = 0;
        while ((j <= n))
        {
          if (((i == 0) || (j == 0)))
          {
            xx[i][j] = 0;
          } else if ((ar[(i - 1)] == br[(j - 1)]))
          {
            xx[i][j] = (xx[(i - 1)][(j - 1)] + 1);
          } else
          {
            if ((xx[(i - 1)][j] > xx[i][(j - 1)]))
            {
              xx[i][j] = xx[(i - 1)][j];
            } else
            {
              xx[i][j] = xx[i][(j - 1)];
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return xx[n][n];
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var ar: dynamic = cpp_array(5002);
  var arr: dynamic = cpp_array(5002);
  var br: dynamic = cpp_array(5002);
  scanf("%d %d", (&n), (&m));
  {
    i = 0;
    while ((i < n))
    {
      scanf("%d %lf", (&ar[i]), (&arr[i]));
      br[i] = ar[i];
      i += 1;
    }
  }
  sort(br, (br + n));
  x = lon_com_sub(ar, br, n);
  printf("%d\n", (n - x));
  return 0;
}
