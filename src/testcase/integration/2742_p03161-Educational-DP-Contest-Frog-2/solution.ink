// Translated from solution.cpp.

var N: dynamic;

var K: dynamic;

var H = cpp_array(100010);

var dp = [0];

func main()
{
  read(N, K);
  {
    var i = 1;
    while ((i <= N))
    {
      read(H[i]);
      dp[i] = 2e9;
      i += 1;
    }
  }
  dp[1] = 0;
  {
    var i = 1;
    while ((i < N))
    {
      {
        var j = 1;
        while ((j <= K))
        {
          dp[(i + j)] = min(dp[(i + j)], (dp[i] + abs((H[i] - H[(i + j)]))));
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(dp[N], "\n");
}
