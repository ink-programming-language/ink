// Translated from solution.cpp.

func main() -> dynamic
{
  var A: dynamic = cpp_array(31, 31);
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    i = 0;
    while ((i < n))
    {
      {
        j = 0;
        while ((j < n))
        {
          scanf("%d", (&A[i][j]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var result: dynamic = 0;
  {
    i = 0;
    while ((i < n))
    {
      {
        j = 0;
        while ((j < n))
        {
          var column: dynamic = 0;
          var c: dynamic = 0;
          {
            while ((c < n))
            {
              column += A[c][j];
              c += 1;
            }
          }
          var row: dynamic = 0;
          var r: dynamic = 0;
          {
            while ((r < n))
            {
              row += A[i][r];
              r += 1;
            }
          }
          if ((column > row))
          {
            result += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d", result);
  return 0;
}
