// Translated from solution.cpp.

var a = cpp_array(5000);

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var cnt = 0;
  var ans = 0;
  while (true)
  {
    {
      var i = 0;
      while ((i < n))
      {
        if ((a[i] <= cnt))
        {
          a[i] = (n + 1);
          cnt += 1;
        }
        i += 1;
      }
    }
    if ((cnt == n))
    {
      break;
    }
    ans += 1;
    {
      var i = (n - 1);
      while ((i >= 0))
      {
        if ((a[i] <= cnt))
        {
          a[i] = (n + 1);
          cnt += 1;
        }
        i -= 1;
      }
    }
    if ((cnt == n))
    {
      break;
    }
    ans += 1;
  }
  write(ans);
  return 0;
}
