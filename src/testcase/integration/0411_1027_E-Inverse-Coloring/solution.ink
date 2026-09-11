// Translated from solution.cpp.

var mod: dynamic = 998244353;

var dpp: dynamic = cpp_array(4, 510, 510);

var dpc: dynamic = cpp_array(4, 510, 510);

var f: dynamic = cpp_array(510);

var g: dynamic = cpp_array(510);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var tp2: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var cnt: dynamic = 0;
  var tp: dynamic = cpp_uninitialized();
  read(n, tp2);
  tp2 -= 1;
  dpp[1][1][1] = 1;
  dpp[1][1][2] = 1;
  {
    i = 2;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= i))
        {
          {
            k = 1;
            while ((k <= j))
            {
              tp = max((k + 1), j);
              dpc[tp][(k + 1)][1] = (((dpc[tp][(k + 1)][1] + dpp[j][k][1])) % mod);
              dpc[tp][(k + 1)][2] = (((dpc[tp][(k + 1)][2] + dpp[j][k][2])) % mod);
              dpc[j][1][2] = (((dpc[j][1][2] + dpp[j][k][1])) % mod);
              dpc[j][1][1] = (((dpc[j][1][1] + dpp[j][k][2])) % mod);
              k += 1;
            }
          }
          j += 1;
        }
      }
      if ((i != n))
      {
        {
          j = 1;
          while ((j <= i))
          {
            {
              k = 1;
              while ((k <= j))
              {
                dpp[j][k][1] = dpc[j][k][1];
                dpp[j][k][2] = dpc[j][k][1];
                k += 1;
              }
            }
            j += 1;
          }
        }
        {
          j = 1;
          while ((j <= n))
          {
            {
              k = 1;
              while ((k <= n))
              {
                dpc[j][k][1] = 0;
                dpc[j][k][2] = 0;
                k += 1;
              }
            }
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= i))
        {
          f[i] = ((((f[i] + dpc[i][j][1]) + dpc[i][j][2])) % mod);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    j = 1;
    while ((j <= n))
    {
      {
        k = 1;
        while ((k <= n))
        {
          dpc[j][k][1] = 0;
          dpc[j][k][2] = 0;
          k += 1;
        }
      }
      j += 1;
    }
  }
  {
    j = 1;
    while ((j <= n))
    {
      {
        k = 1;
        while ((k <= j))
        {
          tp = max((k + 1), j);
          dpc[tp][(k + 1)][1] = (((dpc[tp][(k + 1)][1] + dpp[j][k][1])) % mod);
          dpc[j][1][1] = (((dpc[j][1][1] + dpp[j][k][2])) % mod);
          k += 1;
        }
      }
      j += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= i))
        {
          g[i] = (((g[i] + dpc[i][j][1])) % mod);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= n))
        {
          if (((i * j) <= tp2))
          {
            cnt = (((cnt + (f[i] * g[j]))) % mod);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(cnt, "\n");
  return 0;
}
