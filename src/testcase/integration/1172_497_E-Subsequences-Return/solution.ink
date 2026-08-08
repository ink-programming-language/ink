// Translated from solution.cpp.

var maxN = (3e1 + 5);

var LOG = (6e1 + 5);

var INF = 1e18;

var MOD = (1e9 + 7);

var d = cpp_array(LOG);

class Matrix
{
  var n: dynamic;
  var m: dynamic;
  var M: dynamic = cpp_array(maxN, maxN);
  func Matrix(n: dynamic = 0, m: dynamic = 0)
  {
      n = n;
      m = m;
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
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
  func operator_index(i: dynamic)
  {
      return M[i];
    }
  func operator_multiply(A: dynamic)
  {
      var B = cpp_construct(n, A.m);
      {
        var i = 0;
        while ((i < n))
        {
          {
            var k = 0;
            while ((k < m))
            {
              {
                var j = 0;
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
  func print()
  {
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
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

var K = cpp_array(LOG);

func main()
{
  var START = clock();
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  var k: dynamic;
  scanf("%lld%d", (&n), (&k));
  {
    var i = 0;
    while ((i < LOG))
    {
      K[i] = Matrix((k + 1), (k + 1));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < LOG))
    {
      {
        var j = 0;
        while ((j <= k))
        {
          K[i][j][j] = 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var S = cpp_construct((k + 1), (k + 1));
  var T = cpp_construct((k + 1), (k + 1));
  var R = cpp_construct((k + 1), (k + 1));
  var t = cpp_construct((k + 1), (k + 1));
  {
    var i = 0;
    while ((i < k))
    {
      S[i][((((k + i) - 1)) % k)] = 1;
      T[((((k + i) - 1)) % k)][i] = 1;
      i += 1;
    }
  }
  S[k][k] = cpp_assign(T[k][k], "=", 1);
  {
    var i = 0;
    while ((i <= k))
    {
      K[0][0][i] = cpp_assign(K[0][i][i], "=", 1);
      i += 1;
    }
  }
  var m = n;
  var cnt = 0;
  {
    var i = 1;
    while ((m >= k))
    {
      d[cpp_update(cnt, "++")] = (m % k);
      {
        var j = 0;
        while ((j <= k))
        {
          K[i][j][j] = 1;
          j += 1;
        }
      }
      t = K[(i - 1)];
      {
        var j = 0;
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
    var i = 0;
    while ((i <= k))
    {
      R[i][i] = 1;
      i += 1;
    }
  }
  var sum = 0;
  {
    cnt;
    while ((cnt > -1))
    {
      t = K[cnt];
      {
        var i = 0;
        while ((i < sum))
        {
          t = (((S * t)) * T);
          i += 1;
        }
      }
      {
        var i = 0;
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
  var ans = 0;
  {
    var i = 0;
    while ((i <= k))
    {
      ans = (((ans + R[i][k])) % MOD);
      i += 1;
    }
  }
  printf("%d\n", ans);
  var FINISH = clock();
  write("Execution time: ", ((cpp_cast(((FINISH - START))) / CLOCKS_PER_SEC) * 1000.0), " milliseconds.\n");
  return 0;
}
