// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var tbl = [0];
  tbl[0][0] = 1;
  {
    var i = 1;
    while ((i < 5))
    {
      {
        var j = 0;
        while ((j < 4001))
        {
          {
            var k = 0;
            while ((k < 1001))
            {
              if (((j - k) < 0))
              {
                break;
              }
              tbl[i][j] += tbl[(i - 1)][(j - k)];
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  while ((cin >> n))
  {
    write(tbl[4][n], "\n");
  }
  return 0;
}
