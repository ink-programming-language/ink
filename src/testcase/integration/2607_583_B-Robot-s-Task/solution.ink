// Translated from solution.cpp.

var a: dynamic = cpp_array(5000);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var cnt: dynamic = 0;
  var ans: dynamic = 0;
  while (true)
  {
    {
      var i: dynamic = 0;
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
      var i: dynamic = (n - 1);
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
