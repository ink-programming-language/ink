// Translated from solution.cpp.

var s = cpp_array(155);

var INF = 1e9;

var a = cpp_array(155);

var best = cpp_array(155, 155);

var f = cpp_array(155, 155, 155);

var flag: dynamic;

var l: dynamic;

var r: dynamic;

var L: dynamic;

var i: dynamic;

var j: dynamic;

var k: dynamic;

var p: dynamic;

var m: dynamic;

var n: dynamic;

func upd(x: dynamic)
{
  if ((x > f[i][j][k]))
  {
    f[i][j][k] = x;
  }
}

func main()
{
  scanf("%d", (&n));
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      if ((a[i] == -1))
      {
        a[i] = (-INF);
      }
      i += 1;
    }
  }
  scanf("%s", (s + 1));
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = i;
        while ((j <= n))
        {
          {
            k = 0;
            while ((k <= n))
            {
              f[i][j][k] = (-INF);
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
    L = 1;
    while ((L <= n))
    {
      {
        i = 1;
        while ((i <= ((n - L) + 1)))
        {
          j = ((i + L) - 1);
          if ((L == 1))
          {
            f[i][j][0] = a[1];
          }
          {
            m = i;
            while ((m < j))
            {
              f[i][j][0] = max(f[i][j][0], (f[i][m][0] + f[(m + 1)][j][0]));
              m += 1;
            }
          }
          {
            flag = 1;
            l = i;
            r = j;
            while ((l < r))
            {
              if ((s[l] != s[r]))
              {
                flag = 0;
              }
              l += 1;
              r -= 1;
            }
          }
          if (flag)
          {
            f[i][j][0] = max(f[i][j][0], a[L]);
          }
          {
            m = i;
            while ((m <= j))
            {
              f[i][j][1] = max(f[i][j][1], (f[i][(m - 1)][0] + f[(m + 1)][j][0]));
              m += 1;
            }
          }
          {
            k = 2;
            while ((k <= L))
            {
              {
                p = (i + 1);
                while ((p <= j))
                {
                  upd((f[i][(p - 1)][0] + f[p][j][k]));
                  p += 1;
                }
              }
              {
                p = i;
                while ((p < j))
                {
                  upd((f[(p + 1)][j][0] + f[i][p][k]));
                  p += 1;
                }
              }
              if ((s[i] == s[j]))
              {
                upd(f[(i + 1)][(j - 1)][(k - 2)]);
              }
              k += 1;
            }
          }
          {
            k = 1;
            while ((k <= L))
            {
              f[i][j][0] = max(f[i][j][0], (f[i][j][k] + a[k]));
              k += 1;
            }
          }
          best[i][j] = max(0, f[i][j][0]);
          {
            m = i;
            while ((m < j))
            {
              best[i][j] = max(best[i][j], (best[i][m] + best[(m + 1)][j]));
              m += 1;
            }
          }
          i += 1;
        }
      }
      L += 1;
    }
  }
  printf("%d\n", best[1][n]);
}
