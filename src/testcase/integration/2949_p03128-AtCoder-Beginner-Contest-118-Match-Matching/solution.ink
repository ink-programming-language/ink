// Translated from solution.cpp.

var N: dynamic;

var M: dynamic;

var A = cpp_array(10);

var num = [-1, 2, 5, 5, 4, 5, 6, 3, 7, 6];

var dp = cpp_array(10010);

func main()
{
  read(N, M);
  {
    var i = 0;
    while ((i < M))
    {
      read(A[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= N))
    {
      {
        var j = 0;
        while ((j < M))
        {
          var n = num[A[j]];
          var tmp: dynamic;
          var ss: dynamic;
          if ((i < n))
          {
            j += 1;
            continue;
          }
          if ((i == n))
          {
            (ss << A[j]);
          } else if ((dp[(i - n)] != ""))
          {
            ((ss << A[j]) << dp[(i - n)]);
          } else
          {
            j += 1;
            continue;
          }
          (ss >> tmp);
          if (((tmp.size() > dp[i].size()) || (((tmp.size() == dp[i].size()) && (tmp > dp[i])))))
          {
            dp[i] = tmp;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(dp[N], "\n");
}
