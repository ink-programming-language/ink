// Translated from solution.cpp.

var MOD: dynamic = 1000000007;

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var s: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    var n: dynamic = cpp_uninitialized();
    s.clear();
    read(x, s);
    n = s.size();
    if ((n == 1))
    {
      write(1, "\n");
      continue;
    }
    var done: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i <= x))
      {
        if (done)
        {
          var left: dynamic = i;
          var right: dynamic = ((n - i) + MOD);
          right %= MOD;
          right *= ((s[(i - 1)] - cpp_char("0")));
          right %= MOD;
          n = (left + right);
          n %= MOD;
        } else
        {
          var y: dynamic = s.size();
          if ((s[(i - 1)] == cpp_char("1")))
          {
            i += 1;
            continue;
          }
          var tmp2: dynamic = s.substr(i, y);
          {
            var j: dynamic = 1;
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
