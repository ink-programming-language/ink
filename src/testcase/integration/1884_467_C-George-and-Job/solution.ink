// Translated from solution.cpp.

var N: dynamic = (5e3 + 7);

var mod: dynamic = (1e9 + 7);

var ans: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var d: dynamic = cpp_array(N, N);

var v: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  read(n, m, k);
  var y: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      a[i] = (a[(i - 1)] + x);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      {
        var j: dynamic = m;
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
