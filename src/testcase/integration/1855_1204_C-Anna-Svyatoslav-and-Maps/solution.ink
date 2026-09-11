// Translated from solution.cpp.

var s: dynamic = cpp_array(110, 110);

var a: dynamic = cpp_array(1000010);

var d: dynamic = cpp_array(110, 110);

var dp: dynamic = cpp_array(1000010);

var pv: dynamic = cpp_array(1000010);

func go(x: dynamic) -> dynamic
{
  if ((pv[x] != -1))
  {
    go(pv[x]);
  }
  printf("%d ", (a[x] + 1));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%s", s[i]);
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          d[i][j] =  ((s[i][j] - cpp_char("0"))) ? 1 : 1000000;
          if ((i == j))
          {
            d[i][j] = 0;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var k: dynamic = 0;
    while ((k < n))
    {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          {
            var j: dynamic = 0;
            while ((j < n))
            {
              if ((d[i][j] > (d[i][k] + d[k][j])))
              {
                d[i][j] = (d[i][k] + d[k][j]);
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      k += 1;
    }
  }
  var m: dynamic = cpp_uninitialized();
  scanf("%d", (&m));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d", (&a[i]));
      a[i] -= 1;
      i += 1;
    }
  }
  dp[0] = 1;
  pv[0] = -1;
  {
    var i: dynamic = 1;
    while ((i < m))
    {
      dp[i] = (m + 1);
      {
        var j: dynamic = (i - 1);
        while ((j >= 0))
        {
          if (((i - j) >= n))
          {
            break;
          }
          if (((d[a[j]][a[i]] == (i - j)) && (dp[i] > (dp[j] + 1))))
          {
            dp[i] = (dp[j] + 1);
            pv[i] = j;
          }
          j -= 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", dp[(m - 1)]);
  go((m - 1));
  return 0;
}
