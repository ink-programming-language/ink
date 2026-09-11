// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var K: dynamic = cpp_uninitialized();

var H: dynamic = cpp_array(100010);

var dp: dynamic = [0];

func main() -> dynamic
{
  read(N, K);
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      read(H[i]);
      dp[i] = 2e9;
      i += 1;
    }
  }
  dp[1] = 0;
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      {
        var j: dynamic = 1;
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
