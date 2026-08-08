// Translated from solution.cpp.

var N = 22;

var mod = (1e9 + 7);

var inf = 1e15;

var n: dynamic;

var m: dynamic;

var a = cpp_array(N, N);

var faa = cpp_array(N, N);

var famask = cpp_array(N, N);

var s = cpp_array(N);

var dp = cpp_array((1 << 20));

func smin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func main()
{
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      read(s[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          read(a[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          var faval = 0;
          var ma = 0;
          {
            var k = 0;
            while ((k < n))
            {
              if ((s[i][j] == s[k][j]))
              {
                faval += a[k][j];
                ma = max(ma, a[k][j]);
                famask[i][j] |= ((1 << k));
              }
              k += 1;
            }
          }
          faval -= ma;
          faa[i][j] = faval;
          j += 1;
        }
      }
      i += 1;
    }
  }
  fill((&dp[0]), ((&dp[0]) + ((1 << 20))), inf);
  dp[0] = 0;
  {
    var mask = 0;
    while ((mask < ((1 << n))))
    {
      {
        var j = 0;
        while ((j < n))
        {
          if ((((mask >> j)) & 1))
          {
            j += 1;
            continue;
          }
          {
            var k = 0;
            while ((k < m))
            {
              smin(dp[(mask | ((1 << j)))], (dp[mask] + a[j][k]));
              smin(dp[(mask | (famask[j][k]))], (dp[mask] + faa[j][k]));
              k += 1;
            }
          }
          j += 1;
        }
      }
      mask += 1;
    }
  }
  write(dp[(((1 << n)) - 1)], cpp_char("\n"));
}
