// Translated from solution.cpp.

var mod: dynamic = 1000000007;

var inf: dynamic = 1000000009;

var INF: dynamic = 1000000000000000009;

var big: dynamic = 1000000000000000;

var eps: dynamic = 0.0000000001;

var T: dynamic = cpp_array(100005, 21);

var C: dynamic = cpp_array(100005);

var DP: dynamic = cpp_array(21, ((1 << 20)));

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie();
  cout.tie();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          var c: dynamic = cpp_uninitialized();
          read(c);
          T[i][j] = (c - cpp_char("0"));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      {
        var j: dynamic = 0;
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
  var wynik: dynamic = inf;
  {
    var j: dynamic = 1;
    while ((j <= n))
    {
      {
        var i: dynamic = 0;
        while ((i < ((1 << n))))
        {
          if ((j >= 2))
          {
            DP[i][j] += (cpp_cast((((j - 2) - n))) * DP[i][(j - 2)]);
          }
          {
            var k: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < ((1 << n))))
    {
      var aktual: dynamic = 0;
      {
        var j: dynamic = 0;
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
