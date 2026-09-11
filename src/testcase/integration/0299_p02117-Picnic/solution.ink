// Translated from solution.cpp.

var F: dynamic = cpp_expression("#incl");

var S: dynamic = cpp_expression("#inclu");

var N: dynamic = 14;

var dp1: dynamic = cpp_array(1001, N);

var c: dynamic = cpp_array(N, N);

var dp2: dynamic = cpp_array(N, (1 << N));

var dp3: dynamic = cpp_array(1001, (1 << (((N / 2) + 1))), 2);

var dp: dynamic = cpp_array(1001, (1 << N));

var ans: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array((N + 1));

func main() -> dynamic
{
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(n, A, B);
  {
    var k: dynamic = 0;
    while ((k < n))
    {
      fill(dp1[k], ((dp1[k] + B) + 1), (-((1 << 30))));
      dp1[k][0] = 0;
      var m: dynamic = cpp_uninitialized();
      read(m);
      {
        var i: dynamic = 0;
        while ((i < m))
        {
          var p: dynamic = cpp_uninitialized();
          read(p.S.F, p.F, p.S.S);
          var t: dynamic = 1;
          while (p.S.S)
          {
            while (true)
            {
              {
                var j: dynamic = B;
                while ((j >= 0))
                {
                  if (((j + (p.S.F * t)) <= B))
                  {
                    dp1[k][(j + (p.S.F * t))] = max(dp1[k][(j + (p.S.F * t))], (dp1[k][j] + (p.F * t)));
                  }
                  j -= 1;
                }
              }
              p.S.S -= t;
              if (!(((p.S.S % ((t * 2))))))
              {
                break;
              }
            }
            t *= 2;
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < B))
        {
          dp1[k][(i + 1)] = max(dp1[k][(i + 1)], dp1[k][i]);
          i += 1;
        }
      }
      k += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          read(c[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var k: dynamic = 0;
    while ((k < n))
    {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          {
            var j: dynamic = 0;
            while ((j < n))
            {
              c[i][j] = min(c[i][j], (c[i][k] + c[k][j]));
              j += 1;
            }
          }
          i += 1;
        }
      }
      k += 1;
    }
  }
  {
    var t: dynamic = 0;
    while ((t < ((1 << n))))
    {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          dp2[t][i] = (1 << 30);
          i += 1;
        }
      }
      t += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      dp2[(1 << i)][i] = c[0][i];
      i += 1;
    }
  }
  {
    var t: dynamic = 1;
    while ((t < ((1 << n))))
    {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          if ((!((t & ((1 << i))))))
          {
            i += 1;
            continue;
          }
          {
            var j: dynamic = 0;
            while ((j < n))
            {
              if ((t & ((1 << j))))
              {
                j += 1;
                continue;
              }
              dp2[(t | ((1 << j)))][j] = min(dp2[(t | ((1 << j)))][j], (dp2[t][i] + c[i][j]));
              j += 1;
            }
          }
          i += 1;
        }
      }
      t += 1;
    }
  }
  {
    var t: dynamic = 0;
    while ((t < ((1 << (((n / 2) + (n % 2)))))))
    {
      {
        var i: dynamic = 0;
        while ((i <= B))
        {
          dp3[0][t][i] = cpp_assign(dp3[1][t][i], "=", (-((1 << 30))));
          i += 1;
        }
      }
      t += 1;
    }
  }
  {
    var l: dynamic = 0;
    while ((l < 2))
    {
      dp3[l][0][0] = 0;
      {
        var t: dynamic = 0;
        while ((t < ((1 << (((n / 2) + ((n % 2) * l)))))))
        {
          {
            var i: dynamic = 0;
            while ((i < ((n / 2) + ((n % 2) * l))))
            {
              if ((t & ((1 << i))))
              {
                i += 1;
                continue;
              }
              {
                var j: dynamic = B;
                while ((j >= 0))
                {
                  {
                    var k: dynamic = (B - j);
                    while ((k >= 0))
                    {
                      dp3[l][(t | ((1 << i)))][(j + k)] = max(dp3[l][(t | ((1 << i)))][(j + k)], (dp3[l][t][j] + dp1[(i + ((n / 2) * l))][k]));
                      k -= 1;
                    }
                  }
                  j -= 1;
                }
              }
              i += 1;
            }
          }
          {
            var i: dynamic = 0;
            while ((i < B))
            {
              dp3[l][t][(i + 1)] = max(dp3[l][t][(i + 1)], dp3[l][t][i]);
              i += 1;
            }
          }
          t += 1;
        }
      }
      l += 1;
    }
  }
  {
    var t: dynamic = 0;
    while ((t < ((1 << n))))
    {
      {
        var i: dynamic = 0;
        while ((i <= B))
        {
          dp[t][i] = (-((1 << 30)));
          i += 1;
        }
      }
      t += 1;
    }
  }
  {
    var t: dynamic = 0;
    while ((t < ((1 << ((n / 2))))))
    {
      if (((n > 1) && ((t % 2) == 0)))
      {
        t += 1;
        continue;
      }
      {
        var s: dynamic = 0;
        while ((s < ((1 << (((n / 2) + (n % 2)))))))
        {
          var C: dynamic = max(0, (A - dp2[(t | ((s << ((n / 2)))))][0]));
          var D: dynamic = min(B, C);
          {
            var i: dynamic = 0;
            while ((i <= D))
            {
              ans = max(ans, (dp3[0][t][i] + dp3[1][s][(D - i)]));
              i += 1;
            }
          }
          s += 1;
        }
      }
      t += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
