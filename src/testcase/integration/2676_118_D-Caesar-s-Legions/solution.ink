// Translated from solution.cpp.

var a: dynamic = cpp_array(2, 110, 110);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  var mod: dynamic = 100000000;
  var k: dynamic = cpp_uninitialized();
  var n1: dynamic = cpp_uninitialized();
  var n2: dynamic = cpp_uninitialized();
  var k1: dynamic = cpp_uninitialized();
  var k2: dynamic = cpp_uninitialized();
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
