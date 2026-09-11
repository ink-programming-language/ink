// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var minn: dynamic = 1000000000;
  var minn1: dynamic = 1000000000;
  read(n);
  {
    var i: dynamic = (0);
    while ((i < (n)))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = (1);
    while ((i < ((n - 1))))
    {
      minn = 0;
      {
        var j: dynamic = (1);
        while ((j < (n)))
        {
          if ((!(((i == j) || ((i + 1) == j)))))
          {
            minn = max(minn, (a[j] - a[(j - 1)]));
          }
          j += 1;
        }
      }
      minn = max(minn, (a[(i + 1)] - a[(i - 1)]));
      minn1 = min(minn, minn1);
      i += 1;
    }
  }
  write(minn1, "\n");
  return 0;
}
