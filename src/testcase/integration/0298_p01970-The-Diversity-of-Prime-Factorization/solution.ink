// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var MOD: dynamic = 1000000007;

func sieve_of_eratosthenes(n: dynamic) -> dynamic
{
  {
    var i: dynamic = 2;
    while ((i < n))
    {
      primes[i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
    while (((i * i) < n))
    {
      if (primes[i])
      {
        {
          var j: dynamic = (i * i);
          while ((j < n))
          {
            primes[j] = 0;
            j += i;
          }
        }
      }
      i += 1;
    }
  }
  return primes;
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var primes: dynamic = sieve_of_eratosthenes(1000005);
  var N: dynamic = cpp_uninitialized();
  read(N);
  var res: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(A[i]);
      i += 1;
    }
  }
  var B: dynamic = cpp_uninitialized();
  var C: dynamic = cpp_uninitialized();
  var f1: dynamic = cpp_uninitialized();
  var f2: dynamic = cpp_uninitialized();
  var f3: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if ((i > 0))
      {
        f1 = ((primes[A[(i - 1)]] != 0));
      } else
      {
        f1 = false;
      }
      f2 = ((primes[A[i]] != 0));
      if ((i < (N - 1)))
      {
        f3 = ((primes[A[(i + 1)]] != 0));
      } else
      {
        f3 = true;
      }
      if (((!f2) && (!f3)))
      {
        write(0, "\n");
        return 0;
      }
      if (((!f1) && (!f2)))
      {
        write(0, "\n");
        return 0;
      }
      if (f2)
      {
        if ((!f1))
        {
          B.push_back(A[i]);
          C.push_back(1);
        } else if ((!f3))
        {
          B.push_back(A[i]);
          C.push_back(1);
        } else
        {
          B.push_back(A[i]);
          C.push_back(0);
        }
      }
      i += 1;
    }
  }
  var dp: dynamic = cpp_construct((cpp_cast(B.size()) + 1), vector(3, 0));
  dp[0][1] = 1;
  {
    var i: dynamic = 1;
    while ((i < B.size()))
    {
      if ((C[i] == 0))
      {
        if (((i > 0) && (B[i] > B[(i - 1)])))
        {
          dp[i][1] = dp[(i - 1)][1];
        }
        if (((i <= 1) || (B[i] > B[(i - 2)])))
        {
          dp[i][1] = (((dp[i][1] + dp[(i - 1)][0])) % MOD);
        }
        dp[i][0] = (((dp[i][0] + dp[(i - 1)][1])) % MOD);
      } else
      {
        if (((i > 0) && (B[i] > B[(i - 1)])))
        {
          dp[i][1] = dp[(i - 1)][1];
        }
        if (((i <= 1) || (B[i] > B[(i - 2)])))
        {
          dp[i][1] = (((dp[i][1] + dp[(i - 1)][0])) % MOD);
        }
        dp[i][0] = 0;
      }
      i += 1;
    }
  }
  write((((dp[(B.size() - 1)][0] + dp[(B.size() - 1)][1])) % MOD), "\n");
}
