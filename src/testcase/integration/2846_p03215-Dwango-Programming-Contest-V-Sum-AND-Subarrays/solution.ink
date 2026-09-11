// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  read(N, K);
  var a: dynamic = cpp_construct((N + 1));
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      read(a[i]);
      i += 1;
    }
  }
  var d: dynamic = cpp_construct((N + 1), 0);
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      d[i] = (d[(i - 1)] + a[i]);
      i += 1;
    }
  }
  var v: dynamic = cpp_uninitialized();
  {
    var l: dynamic = 0;
    while ((l <= (N - 1)))
    {
      {
        var r: dynamic = (l + 1);
        while ((r <= N))
        {
          var x: dynamic = (d[r] - d[l]);
          v.push_back(x);
          r += 1;
        }
      }
      l += 1;
    }
  }
  var res: dynamic = 0;
  {
    var i: dynamic = 50;
    while ((i >= 1))
    {
      var x: dynamic = (res + pow(2, (i - 1)));
      var cnt: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((j < v.size()))
        {
          var y: dynamic = (x & v[j]);
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
