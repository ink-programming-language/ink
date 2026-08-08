// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var res: dynamic;

var c = cpp_array(2005, 2005);

func main()
{
  read(n, m);
  var p: dynamic;
  var q: dynamic;
  var s: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      p = 1;
      q = 0;
      {
        var j = 2;
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
    var i = 1;
    while ((i < n))
    {
      p = 1;
      q = 0;
      {
        var j = 2;
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
    var j = 2;
    while ((j <= m))
    {
      s = ((m - j) + 1);
      res = (((res + (s * c[n][j]))) % 1000000007);
      j += 1;
    }
  }
  write(res);
}
