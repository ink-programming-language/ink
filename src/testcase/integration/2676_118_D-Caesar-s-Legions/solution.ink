// Translated from solution.cpp.

var a = cpp_array(2, 110, 110);

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var i: dynamic;
  var j: dynamic;
  var ans = 0;
  var mod = 100000000;
  var k: dynamic;
  var n1: dynamic;
  var n2: dynamic;
  var k1: dynamic;
  var k2: dynamic;
  read(n1, n2, k1, k2);
  a[0][0][0] = 1;
  a[0][0][1] = 1;
  {
    i = 0;
    while ((i <= n1))
    {
      {
        j = 0;
        while ((j <= n2))
        {
          {
            k = 1;
            while ((k <= min(k1, i)))
            {
              a[i][j][0] += a[(i - k)][j][1];
              a[i][j][0] = (a[i][j][0] % mod);
              k += 1;
            }
          }
          {
            k = 1;
            while ((k <= min(k2, j)))
            {
              a[i][j][1] += a[i][(j - k)][0];
              a[i][j][1] = (a[i][j][1] % mod);
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write((((a[n1][n2][0] + a[n1][n2][1])) % mod));
  return 0;
}
