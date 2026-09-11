// Translated from solution.cpp.

var s: dynamic = cpp_array(155);

var INF: dynamic = 1e9;

var a: dynamic = cpp_array(155);

var best: dynamic = cpp_array(155, 155);

var f: dynamic = cpp_array(155, 155, 155);

var flag: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var L: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

func upd(x: dynamic) -> dynamic
{
  if ((x > f[i][j][k]))
  {
    f[i][j][k] = x;
  }
}

func main() -> dynamic
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
