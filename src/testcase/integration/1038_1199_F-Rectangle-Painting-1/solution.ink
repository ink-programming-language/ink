// Translated from solution.cpp.

var n: dynamic;

var s = cpp_array(51, 51);

var ans = cpp_array(51, 51, 51, 51);

func solve(r1: dynamic, c1: dynamic, r2: dynamic, c2: dynamic)
{
  if (((r1 == r2) && (c1 == c2)))
  {
    ans[r1][c1][r2][c2] = ((s[r1][c1] == cpp_char("#")));
  }
  if ((ans[r1][c1][r2][c2] != -1))
  {
    return ans[r1][c1][r2][c2];
  }
  ans[r1][c1][r2][c2] = max(((r2 - r1) + 1), ((c2 - c1) + 1));
  var cans = ans[r1][c1][r2][c2];
  {
    var i = (r1 + 1);
    while ((i <= r2))
    {
      cans = min(cans, (solve(r1, c1, (i - 1), c2) + solve(i, c1, r2, c2)));
      i += 1;
    }
  }
  {
    var i = (c1 + 1);
    while ((i <= c2))
    {
      cans = min(cans, (solve(r1, c1, r2, (i - 1)) + solve(r1, i, r2, c2)));
      i += 1;
    }
  }
  ans[r1][c1][r2][c2] = cans;
  return ans[r1][c1][r2][c2];
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%s", s[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          {
            var k = 0;
            while ((k < n))
            {
              {
                var l = 0;
                while ((l < n))
                {
                  ans[i][j][k][l] = -1;
                  l += 1;
                }
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = solve(0, 0, (n - 1), (n - 1));
  printf("%d\n", ans);
}
