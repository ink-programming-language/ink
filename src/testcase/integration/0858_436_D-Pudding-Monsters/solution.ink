// Translated from solution.cpp.

var MAXN = 100005;

var MAXM = 2005;

var N: dynamic;

var M: dynamic;

var mon = cpp_array(MAXN);

var cell = cpp_array(MAXM);

var start = cpp_array(MAXN);

var use = cpp_array(MAXN);

var dp = cpp_array(MAXN);

func main()
{
  ios.sync_with_stdio(false);
  read(N, M);
  {
    var i = 0;
    while ((i < N))
    {
      read(mon[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < M))
    {
      read(cell[i]);
      i += 1;
    }
  }
  sort(mon, (mon + N));
  sort(cell, (cell + M));
  {
    var i = 0;
    while ((i < N))
    {
      if (((i == 0) || (mon[i] != (mon[(i - 1)] + 1))))
      {
        start[i] = i;
      } else
      {
        start[i] = start[(i - 1)];
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      var next = (upper_bound(cell, (cell + M), mon[i]) - cell);
      if ((start[i] > 0))
      {
        use[i] = max(use[i], dp[(start[i] - 1)]);
      }
      if ((i > 0))
      {
        dp[i] = max(dp[i], dp[(i - 1)]);
      }
      {
        var j = (next - 1);
        while ((j >= 0))
        {
          var left = (i - ((mon[i] - cell[j])));
          if ((left < 0))
          {
            break;
          }
          use[i] = max(use[i], ((dp[(start[left] - 1)] + next) - j));
          j -= 1;
        }
      }
      {
        var j = next;
        while ((j < M))
        {
          var right = (i + ((cell[j] - mon[i])));
          if ((right >= N))
          {
            break;
          }
          dp[right] = max(dp[right], (((use[i] + j) - next) + 1));
          j += 1;
        }
      }
      dp[i] = max(dp[i], use[i]);
      i += 1;
    }
  }
  write(dp[(N - 1)], "\n");
  return 0;
}
