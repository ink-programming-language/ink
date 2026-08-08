// Translated from solution.cpp.

var MOD = 1000000007;

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var s: dynamic;
    var x: dynamic;
    var n: dynamic;
    s.clear();
    read(x, s);
    n = s.size();
    if ((n == 1))
    {
      write(1, "\n");
      continue;
    }
    var done = 0;
    {
      var i = 1;
      while ((i <= x))
      {
        if (done)
        {
          var left = i;
          var right = ((n - i) + MOD);
          right %= MOD;
          right *= ((s[(i - 1)] - cpp_char("0")));
          right %= MOD;
          n = (left + right);
          n %= MOD;
        } else
        {
          var y = s.size();
          if ((s[(i - 1)] == cpp_char("1")))
          {
            i += 1;
            continue;
          }
          var tmp2 = s.substr(i, y);
          {
            var j = 1;
            while ((j < ((s[(i - 1)] - cpp_char("0")))))
            {
              s += tmp2;
              j += 1;
            }
          }
          n = s.size();
          if ((n > x))
          {
            done = 1;
          }
          if ((i > n))
          {
            break;
          }
          n %= MOD;
        }
        i += 1;
      }
    }
    write((n % MOD), "\n");
  }
}
