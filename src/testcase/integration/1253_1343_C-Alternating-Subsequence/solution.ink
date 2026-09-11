// Translated from solution.cpp.

func main(argument_0: dynamic) -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  {
    var j: dynamic = 0;
    while ((j < t))
    {
      var n: dynamic = cpp_uninitialized();
      read(n);
      var ans: dynamic = 0;
      var last: dynamic = cpp_uninitialized();
      var x: dynamic = cpp_uninitialized();
      read(x);
      last = x;
      if ((n == 1))
      {
        ans += last;
      }
      {
        var i: dynamic = 2;
        while ((i <= n))
        {
          read(x);
          if (cpp_binary((last > 0), "and", (x > 0)))
          {
            last = max(last, x);
            if ((i == n))
            {
              ans += last;
            }
          } else if (cpp_binary((last > 0), "and", (x < 0)))
          {
            ans += last;
            last = x;
            if ((i == n))
            {
              ans += last;
            }
          } else if (cpp_binary((last < 0), "and", (x < 0)))
          {
            last = max(last, x);
            if ((i == n))
            {
              ans += last;
            }
          } else if (cpp_binary((last < 0), "and", (x > 0)))
          {
            ans += last;
            last = x;
            if ((i == n))
            {
              ans += last;
            }
          }
          i += 1;
        }
      }
      write(ans, "\n");
      j += 1;
    }
  }
  return 0;
}
