// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var res: dynamic = cpp_uninitialized();

var c: dynamic = cpp_array(2005, 2005);

func main() -> dynamic
{
  read(n, m);
  var p: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      p = 1;
      q = 0;
      {
        var j: dynamic = 2;
        while ((j <= m))
        {
          q = (((q + c[(i - 1)][j])) % 1000000007);
          p = (((p + q)) % 1000000007);
          c[i][j] = p;
          j += 1;
        }
      }
      i += 1;
    }
  }
  res = 0;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      p = 1;
      q = 0;
      {
        var j: dynamic = 2;
        while ((j <= m))
        {
          s = ((m - j) + 1);
          p = (((p + q)) % 1000000007);
          q = (((q + c[(n - i)][j])) % 1000000007);
          res = (((res + (s * (((c[i][j] * p) % 1000000007))))) % 1000000007);
          p = (((p + c[(n - i)][j])) % 1000000007);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var j: dynamic = 2;
    while ((j <= m))
    {
      s = ((m - j) + 1);
      res = (((res + (s * c[n][j]))) % 1000000007);
      j += 1;
    }
  }
  write(res);
}
