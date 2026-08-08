// Translated from solution.cpp.

var s = cpp_array(110, 110);

var a = cpp_array(1000010);

var d = cpp_array(110, 110);

var dp = cpp_array(1000010);

var pv = cpp_array(1000010);

func go(x: dynamic)
{
  if ((pv[x] != -1))
  {
    go(pv[x]);
  }
  printf("%d ", (a[x] + 1));
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%s", s[i]);
      {
        var j = 0;
        while ((j < n))
        {
          d[i][j] = if ((s[i][j] - cpp_char("0"))) 1 else 1000000;
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
    var k = 0;
    while ((k < n))
    {
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
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
  var m: dynamic;
  scanf("%d", (&m));
  {
    var i = 0;
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
    var i = 1;
    while ((i < m))
    {
      dp[i] = (m + 1);
      {
        var j = (i - 1);
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
