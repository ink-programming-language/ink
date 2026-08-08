// Translated from solution.cpp.

var USE_MATH_DEFINES = cpp_expression("#def");

var MOD = (1e9 + 7);

var dp = cpp_array(5000, 2);

func main()
{
  var N: dynamic;
  var L: dynamic;
  var X = cpp_array(5000);
  var A = cpp_array(5000);
  read(N, L);
  {
    var i = 0;
    while ((i < N))
    {
      read(X[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      read(A[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      {
        var j = 0;
        while (((X[i] + (A[i] * j)) < L))
        {
          dp[(i & 1)][(X[i] + (A[i] * j))] = 1;
          if ((A[i] == 0))
          {
            break;
          }
          j += 1;
        }
      }
      if ((i == 0))
      {
        i += 1;
        continue;
      }
      var sum = 0;
      {
        var j = 0;
        while ((j < L))
        {
          dp[(i & 1)][j] = (((dp[(i & 1)][j] * sum)) % MOD);
          sum += (dp[(((i - 1)) & 1)][j] % MOD);
          dp[(((i - 1)) & 1)][j] = 0;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < L))
    {
      ans += (dp[(((N - 1)) & 1)][i] % MOD);
      i += 1;
    }
  }
  printf("%lld\n", (ans % MOD));
  return 0;
}
