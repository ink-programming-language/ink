// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var tbl: dynamic = [0];
  tbl[0][0] = 1;
  {
    var i: dynamic = 1;
    while ((i < 5))
    {
      {
        var j: dynamic = 0;
        while ((j < 4001))
        {
          {
            var k: dynamic = 0;
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
