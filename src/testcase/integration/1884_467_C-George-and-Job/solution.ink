// Translated from solution.cpp.

var N = (5e3 + 7);

var mod = (1e9 + 7);

var ans: dynamic;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var a = cpp_array(N);

var d = cpp_array(N, N);

var v: dynamic;

func main()
{
  ios_base.sync_with_stdio(0);
  read(n, m, k);
  var y = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      read(x);
      a[i] = (a[(i - 1)] + x);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= k))
    {
      {
        var j = m;
        while ((j <= n))
        {
          d[i][j] = max(d[i][(j - 1)], ((a[j] - a[(j - m)]) + d[(i - 1)][(j - m)]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(d[k][n]);
}
