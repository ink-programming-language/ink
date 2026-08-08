// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  var K: dynamic;
  read(N, K);
  var a = cpp_construct((N + 1));
  {
    var i = 1;
    while ((i <= N))
    {
      read(a[i]);
      i += 1;
    }
  }
  var d = cpp_construct((N + 1), 0);
  {
    var i = 1;
    while ((i <= N))
    {
      d[i] = (d[(i - 1)] + a[i]);
      i += 1;
    }
  }
  var v: dynamic;
  {
    var l = 0;
    while ((l <= (N - 1)))
    {
      {
        var r = (l + 1);
        while ((r <= N))
        {
          var x = (d[r] - d[l]);
          v.push_back(x);
          r += 1;
        }
      }
      l += 1;
    }
  }
  var res = 0;
  {
    var i = 50;
    while ((i >= 1))
    {
      var x = (res + pow(2, (i - 1)));
      var cnt = 0;
      {
        var j = 0;
        while ((j < v.size()))
        {
          var y = (x & v[j]);
          if ((y == x))
          {
            cnt += 1;
          }
          j += 1;
        }
      }
      if ((cnt >= K))
      {
        res += pow(2, (i - 1));
      }
      i -= 1;
    }
  }
  write(res, "\n");
  return 0;
}
