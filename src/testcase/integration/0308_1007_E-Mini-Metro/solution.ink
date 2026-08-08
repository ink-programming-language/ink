// Translated from solution.cpp.

var N = 201;

var T = 201;

var n: dynamic;

var t: dynamic;

var z: dynamic;

var a = cpp_array(N);

var b = cpp_array(N);

var c = cpp_array(N);

var pa = cpp_array(N);

var pb = cpp_array(N);

var cl1 = cpp_array(T, N);

var cl2 = cpp_array(T, N);

var mr1 = cpp_array(T, N);

var mr2 = cpp_array(T, N);

var dp1 = cpp_array(T, N);

var dp2 = cpp_array(T, N);

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(n, t, z);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i], b[i], c[i]);
      pa[i] = (pa[(i - 1)] + a[i]);
      pb[i] = (pb[(i - 1)] + b[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= t))
    {
      cl1[0][i] = cpp_assign(cl2[0][i], "=", true);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      cl2[i][0] = true;
      mr2[i][0] = 0;
      {
        var j = 1;
        while ((j <= t))
        {
          mr2[i][j] = -1;
          {
            var k = 0;
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
              var mn = (mr2[i][k] % z);
              var mx = mr2[i][k];
              var rm = ((mn + (pb[i] * ((j - k)))) - ((((((pb[(i - 1)] * ((j - k))) + z) - 1)) / z) * z));
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
                var f = (mx - c[i]);
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
        var j = 1;
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
            var k = 0;
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
        var j = 0;
        while ((j <= t))
        {
          mr1[i][j] = -1;
          if (cl1[(i - 1)][j])
          {
            if (((a[i] + (b[i] * j)) <= c[i]))
            {
              var rm = ((pa[i] + (pb[i] * j)) - ((((((pa[(i - 1)] + (pb[(i - 1)] * j)) + z) - 1)) / z) * z));
              if ((rm >= 0))
              {
                mr1[i][j] = rm;
              }
            }
          }
          {
            var k = 0;
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
              var mn = (mr1[i][k] % z);
              var mx = mr1[i][k];
              var rm = ((mn + (pb[i] * ((j - k)))) - ((((((pb[(i - 1)] * ((j - k))) + z) - 1)) / z) * z));
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
                var f = (mx - c[i]);
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
        var j = 0;
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
            var k = 0;
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
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
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
            var k = 1;
            while ((k <= j))
            {
              if (cl2[i][k])
              {
                var cost = (((((pb[i] * k) + z) - 1)) / z);
                dp2[i][j] = min(dp2[i][j], (dp2[i][(j - k)] + cost));
              }
              k += 1;
            }
          }
          {
            var k = 1;
            while ((k <= j))
            {
              if ((mr2[i][k] != -1))
              {
                if ((((mr2[i][k] % z) + (((j - k)) * b[i])) > c[i]))
                {
                  k += 1;
                  continue;
                }
                var cost = ((((pb[i] * k) - mr2[i][k])) / z);
                var f = ((mr2[i][k] + (((j - k)) * b[i])) - c[i]);
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
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 0;
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
            var k = 0;
            while ((k <= j))
            {
              if (cl1[i][k])
              {
                var cost = ((((((pb[i] * k) + pa[i]) + z) - 1)) / z);
                dp1[i][j] = min(dp1[i][j], (dp2[i][(j - k)] + cost));
              }
              k += 1;
            }
          }
          {
            var k = 0;
            while ((k <= j))
            {
              if ((mr1[i][k] != -1))
              {
                if ((((mr1[i][k] % z) + (((j - k)) * b[i])) > c[i]))
                {
                  k += 1;
                  continue;
                }
                var cost = ((((pa[i] + (pb[i] * k)) - mr1[i][k])) / z);
                var f = ((mr1[i][k] + (((j - k)) * b[i])) - c[i]);
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
