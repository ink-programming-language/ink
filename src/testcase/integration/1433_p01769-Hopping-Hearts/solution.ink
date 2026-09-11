// Translated from solution.cpp.

var USE_MATH_DEFINES: dynamic = cpp_expression("#def");

var MOD: dynamic = (1e9 + 7);

var dp: dynamic = cpp_array(5000, 2);

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var L: dynamic = cpp_uninitialized();
  var X: dynamic = cpp_array(5000);
  var A: dynamic = cpp_array(5000);
  read(N, L);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(X[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(A[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      {
        var j: dynamic = 0;
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
      var sum: dynamic = 0;
      {
        var j: dynamic = 0;
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
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < L))
    {
      ans += (dp[(((N - 1)) & 1)][i] % MOD);
      i += 1;
    }
  }
  printf("%lld\n", (ans % MOD));
  return 0;
}
