// Translated from solution.cpp.

var mod = 1000000007;

var inf = 1000000009;

var INF = 1000000000000000009;

var big = 1000000000000000;

var eps = 0.0000000001;

var T = cpp_array(100005, 21);

var C = cpp_array(100005);

var DP = cpp_array(21, ((1 << 20)));

func main()
{
  ios.sync_with_stdio(false);
  cin.tie();
  cout.tie();
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 1;
        while ((j <= m))
        {
          var c: dynamic;
          read(c);
          T[i][j] = (c - cpp_char("0"));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      {
        var j = 0;
        while ((j < n))
        {
          if (T[j][i])
          {
            C[i] += ((1 << j));
          }
          j += 1;
        }
      }
      DP[C[i]][0] += 1;
      i += 1;
    }
  }
  var wynik = inf;
  {
    var j = 1;
    while ((j <= n))
    {
      {
        var i = 0;
        while ((i < ((1 << n))))
        {
          if ((j >= 2))
          {
            DP[i][j] += (cpp_cast((((j - 2) - n))) * DP[i][(j - 2)]);
          }
          {
            var k = 0;
            while ((k < n))
            {
              DP[i][j] += DP[(i ^ ((1 << k)))][(j - 1)];
              k += 1;
            }
          }
          DP[i][j] /= j;
          i += 1;
        }
      }
      j += 1;
    }
  }
  {
    var i = 0;
    while ((i < ((1 << n))))
    {
      var aktual = 0;
      {
        var j = 0;
        while ((j <= n))
        {
          aktual += (DP[i][j] * min(j, (n - j)));
          j += 1;
        }
      }
      wynik = min(wynik, aktual);
      i += 1;
    }
  }
  write(wynik);
  return 0;
}
