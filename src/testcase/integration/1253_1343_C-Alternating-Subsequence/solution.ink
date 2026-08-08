// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic;
  read(t);
  {
    var j = 0;
    while ((j < t))
    {
      var n: dynamic;
      read(n);
      var ans = 0;
      var last: dynamic;
      var x: dynamic;
      read(x);
      last = x;
      if ((n == 1))
      {
        ans += last;
      }
      {
        var i = 2;
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
