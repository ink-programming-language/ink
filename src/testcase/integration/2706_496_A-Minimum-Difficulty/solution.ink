// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var minn = 1000000000;
  var minn1 = 1000000000;
  read(n);
  {
    var i = (0);
    while ((i < (n)))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = (1);
    while ((i < ((n - 1))))
    {
      minn = 0;
      {
        var j = (1);
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
