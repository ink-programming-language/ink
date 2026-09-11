// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array(10);

var num: dynamic = [-1, 2, 5, 5, 4, 5, 6, 3, 7, 6];

var dp: dynamic = cpp_array(10010);

func main() -> dynamic
{
  read(N, M);
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      read(A[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      {
        var j: dynamic = 0;
        while ((j < M))
        {
          var n: dynamic = num[A[j]];
          var tmp: dynamic = cpp_uninitialized();
          var ss: dynamic = cpp_uninitialized();
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
