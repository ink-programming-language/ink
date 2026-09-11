// Translated from solution.cpp.

var N: dynamic = 201;

var T: dynamic = 201;

var n: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var z: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var b: dynamic = cpp_array(N);

var c: dynamic = cpp_array(N);

var pa: dynamic = cpp_array(N);

var pb: dynamic = cpp_array(N);

var cl1: dynamic = cpp_array(T, N);

var cl2: dynamic = cpp_array(T, N);

var mr1: dynamic = cpp_array(T, N);

var mr2: dynamic = cpp_array(T, N);

var dp1: dynamic = cpp_array(T, N);

var dp2: dynamic = cpp_array(T, N);

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(n, t, z);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i], b[i], c[i]);
      pa[i] = (pa[(i - 1)] + a[i]);
      pb[i] = (pb[(i - 1)] + b[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= t))
    {
      cl1[0][i] = cpp_assign(cl2[0][i], "=", true);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      cl2[i][0] = true;
      mr2[i][0] = 0;
      {
        var j: dynamic = 1;
        while ((j <= t))
        {
          mr2[i][j] = -1;
          {
            var k: dynamic = 0;
            while ((k < j))
            {
              if ((mr2[i][k] == -1))
              {
                k += 1;
                continue;
              }
              if ((!cl2[(i - 1)][(j - k)]))
              {
                k += 1;
                continue;
              }
              var mn: dynamic = (mr2[i][k] % z);
              var mx: dynamic = mr2[i][k];
              var rm: dynamic = ((mn + (pb[i] * ((j - k)))) - ((((((pb[(i - 1)] * ((j - k))) + z) - 1)) / z) * z));
              mn += (((j - k)) * b[i]);
              mx += (((j - k)) * b[i]);
              if ((rm < 0))
              {
                mn += z;
                rm += z;
              }
              if ((mn > mx))
              {
                k += 1;
                continue;
              }
              if ((mn > c[i]))
              {
                k += 1;
                continue;
              }
              if ((mx > c[i]))
              {
                var f: dynamic = (mx - c[i]);
                f = ((((f + z) - 1)) / z);
                mx -= (f * z);
              }
              mx = ((mx - mn) + rm);
              mn = rm;
              mr2[i][j] = max(mr2[i][j], mx);
              k += 1;
            }
          }
          j += 1;
        }
      }
      {
        var j: dynamic = 1;
        while ((j <= t))
        {
          if (cl2[(i - 1)][j])
          {
            if (((b[i] * j) <= c[i]))
            {
              cl2[i][j] = true;
            }
          }
          {
            var k: dynamic = 0;
            while ((k < j))
            {
              if ((mr2[i][k] == -1))
              {
                k += 1;
                continue;
              }
              if ((!cl2[(i - 1)][(j - k)]))
              {
                k += 1;
                continue;
              }
              if ((((mr2[i][k] % z) + (((j - k)) * b[i])) <= c[i]))
              {
                cl2[i][j] = true;
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j <= t))
        {
          mr1[i][j] = -1;
          if (cl1[(i - 1)][j])
          {
            if (((a[i] + (b[i] * j)) <= c[i]))
            {
              var rm: dynamic = ((pa[i] + (pb[i] * j)) - ((((((pa[(i - 1)] + (pb[(i - 1)] * j)) + z) - 1)) / z) * z));
              if ((rm >= 0))
              {
                mr1[i][j] = rm;
              }
            }
          }
          {
            var k: dynamic = 0;
            while ((k < j))
            {
              if ((mr1[i][k] == -1))
              {
                k += 1;
                continue;
              }
              if ((!cl2[(i - 1)][(j - k)]))
              {
                k += 1;
                continue;
              }
              var mn: dynamic = (mr1[i][k] % z);
              var mx: dynamic = mr1[i][k];
              var rm: dynamic = ((mn + (pb[i] * ((j - k)))) - ((((((pb[(i - 1)] * ((j - k))) + z) - 1)) / z) * z));
              mn += (((j - k)) * b[i]);
              mx += (((j - k)) * b[i]);
              if ((rm < 0))
              {
                mn += z;
                rm += z;
              }
              if ((mn > mx))
              {
                k += 1;
                continue;
              }
              if ((mn > c[i]))
              {
                k += 1;
                continue;
              }
              if ((mx > c[i]))
              {
                var f: dynamic = (mx - c[i]);
                f = ((((f + z) - 1)) / z);
                mx -= (f * z);
              }
              mx = ((mx - mn) + rm);
              mn = rm;
              mr1[i][j] = max(mr1[i][j], mx);
              k += 1;
            }
          }
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j <= t))
        {
          if (cl1[(i - 1)][j])
          {
            if (((a[i] + (b[i] * j)) <= c[i]))
            {
              cl1[i][j] = true;
            }
          }
          {
            var k: dynamic = 0;
            while ((k < j))
            {
              if ((mr1[i][k] == -1))
              {
                k += 1;
                continue;
              }
              if ((!cl2[(i - 1)][(j - k)]))
              {
                k += 1;
                continue;
              }
              if ((((mr1[i][k] % z) + (((j - k)) * b[i])) <= c[i]))
              {
                cl1[i][j] = true;
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= t))
        {
          dp2[i][j] = 1e18;
          if (((j * b[i]) <= c[i]))
          {
            dp2[i][j] = dp2[(i - 1)][j];
            j += 1;
            continue;
          }
          {
            var k: dynamic = 1;
            while ((k <= j))
            {
              if (cl2[i][k])
              {
                var cost: dynamic = (((((pb[i] * k) + z) - 1)) / z);
                dp2[i][j] = min(dp2[i][j], (dp2[i][(j - k)] + cost));
              }
              k += 1;
            }
          }
          {
            var k: dynamic = 1;
            while ((k <= j))
            {
              if ((mr2[i][k] != -1))
              {
                if ((((mr2[i][k] % z) + (((j - k)) * b[i])) > c[i]))
                {
                  k += 1;
                  continue;
                }
                var cost: dynamic = ((((pb[i] * k) - mr2[i][k])) / z);
                var f: dynamic = ((mr2[i][k] + (((j - k)) * b[i])) - c[i]);
                if ((f > 0))
                {
                  cost += ((((f + z) - 1)) / z);
                }
                dp2[i][j] = min(dp2[i][j], (dp2[(i - 1)][(j - k)] + cost));
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 0;
        while ((j <= t))
        {
          dp1[i][j] = 1e18;
          if (((a[i] + (j * b[i])) <= c[i]))
          {
            dp1[i][j] = dp1[(i - 1)][j];
            j += 1;
            continue;
          }
          {
            var k: dynamic = 0;
            while ((k <= j))
            {
              if (cl1[i][k])
              {
                var cost: dynamic = ((((((pb[i] * k) + pa[i]) + z) - 1)) / z);
                dp1[i][j] = min(dp1[i][j], (dp2[i][(j - k)] + cost));
              }
              k += 1;
            }
          }
          {
            var k: dynamic = 0;
            while ((k <= j))
            {
              if ((mr1[i][k] != -1))
              {
                if ((((mr1[i][k] % z) + (((j - k)) * b[i])) > c[i]))
                {
                  k += 1;
                  continue;
                }
                var cost: dynamic = ((((pa[i] + (pb[i] * k)) - mr1[i][k])) / z);
                var f: dynamic = ((mr1[i][k] + (((j - k)) * b[i])) - c[i]);
                if ((f > 0))
                {
                  cost += ((((f + z) - 1)) / z);
                }
                dp1[i][j] = min(dp1[i][j], (dp2[(i - 1)][(j - k)] + cost));
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(dp1[n][t], cpp_char("\n"));
}
