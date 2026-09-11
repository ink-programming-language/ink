// Translated from solution.cpp.

var maxN: dynamic = (3e1 + 5);

var LOG: dynamic = (6e1 + 5);

var INF: dynamic = 1e18;

var MOD: dynamic = (1e9 + 7);

var d: dynamic = cpp_array(LOG);

class Matrix
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_array(maxN, maxN);
  func Matrix(n: dynamic = 0, m: dynamic = 0) -> dynamic
  {
      n = n;
      m = m;
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          {
            var j: dynamic = 0;
            while ((j < m))
            {
              M[i][j] = 0;
              j += 1;
            }
          }
          i += 1;
        }
      }
      return;
    }
  func operator_index(i: dynamic) -> dynamic
  {
      return M[i];
    }
  func operator_multiply(A: dynamic) -> dynamic
  {
      var B: dynamic = cpp_construct(n, A.m);
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          {
            var k: dynamic = 0;
            while ((k < m))
            {
              {
                var j: dynamic = 0;
                while ((j < A.m))
                {
                  B[i][j] = ((((0 + B[i][j]) + (((1 * M[i][k]) * A.M[k][j]) % MOD))) % MOD);
                  j += 1;
                }
              }
              k += 1;
            }
          }
          i += 1;
        }
      }
      return B;
    }
  func print() -> dynamic
  {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          {
            var j: dynamic = 0;
            while ((j < m))
            {
              printf("%d ", M[i][j]);
              j += 1;
            }
          }
          i += 1;
          printf("\n");
        }
      }
      printf("\n");
      return;
    }
}

var K: dynamic = cpp_array(LOG);

func main() -> dynamic
{
  var START: dynamic = clock();
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  scanf("%lld%d", (&n), (&k));
  {
    var i: dynamic = 0;
    while ((i < LOG))
    {
      K[i] = Matrix((k + 1), (k + 1));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < LOG))
    {
      {
        var j: dynamic = 0;
        while ((j <= k))
        {
          K[i][j][j] = 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var S: dynamic = cpp_construct((k + 1), (k + 1));
  var T: dynamic = cpp_construct((k + 1), (k + 1));
  var R: dynamic = cpp_construct((k + 1), (k + 1));
  var t: dynamic = cpp_construct((k + 1), (k + 1));
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      S[i][((((k + i) - 1)) % k)] = 1;
      T[((((k + i) - 1)) % k)][i] = 1;
      i += 1;
    }
  }
  S[k][k] = cpp_assign(T[k][k], "=", 1);
  {
    var i: dynamic = 0;
    while ((i <= k))
    {
      K[0][0][i] = cpp_assign(K[0][i][i], "=", 1);
      i += 1;
    }
  }
  var m: dynamic = n;
  var cnt: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((m >= k))
    {
      d[cpp_update(cnt, "++")] = (m % k);
      {
        var j: dynamic = 0;
        while ((j <= k))
        {
          K[i][j][j] = 1;
          j += 1;
        }
      }
      t = K[(i - 1)];
      {
        var j: dynamic = 0;
        while ((j < k))
        {
          K[i] = (t * K[i]);
          t = (((S * t)) * T);
          j += 1;
        }
      }
      m /= k;
      i += 1;
    }
  }
  d[cnt] = m;
  {
    var i: dynamic = 0;
    while ((i <= k))
    {
      R[i][i] = 1;
      i += 1;
    }
  }
  var sum: dynamic = 0;
  {
    cnt;
    while ((cnt > -1))
    {
      t = K[cnt];
      {
        var i: dynamic = 0;
        while ((i < sum))
        {
          t = (((S * t)) * T);
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < d[cnt]))
        {
          R = (t * R);
          t = (((S * t)) * T);
          i += 1;
        }
      }
      sum = (((sum + d[cnt])) % k);
      cnt -= 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= k))
    {
      ans = (((ans + R[i][k])) % MOD);
      i += 1;
    }
  }
  printf("%d\n", ans);
  var FINISH: dynamic = clock();
  write("Execution time: ", ((cpp_cast(((FINISH - START))) / CLOCKS_PER_SEC) * 1000.0), " milliseconds.\n");
  return 0;
}
