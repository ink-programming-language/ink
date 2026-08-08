// Translated from solution.cpp.

func main()
{
  var A = cpp_array(31, 31);
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
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
  var result = 0;
  {
    i = 0;
    while ((i < n))
    {
      {
        j = 0;
        while ((j < n))
        {
          var column = 0;
          var c = 0;
          {
            while ((c < n))
            {
              column += A[c][j];
              c += 1;
            }
          }
          var row = 0;
          var r = 0;
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
