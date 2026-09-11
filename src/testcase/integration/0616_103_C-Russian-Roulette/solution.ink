// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  read(n, k, p);
  while (cpp_update(p, "--"))
  {
    var x: dynamic = cpp_uninitialized();
    read(x);
    if ((k == 0))
    {
      write(cpp_char("."));
      continue;
    } else
    {
      if ((n % 2))
      {
        if ((x == n))
        {
          write(cpp_char("X"));
        } else
        {
          var num_even: dynamic = min((k - 1), (((n - 1)) / 2));
          var num_odd: dynamic = (((k - 1)) - num_even);
          if (((x % 2) == 0))
          {
            var dist: dynamic = ((((n - 1) - x)) / 2);
            if (((dist + 1) <= num_even))
            {
              write(cpp_char("X"));
            } else
            {
              write(cpp_char("."));
            }
          } else
          {
            var dist: dynamic = ((((n - 2) - x)) / 2);
            if (((dist + 1) <= num_odd))
            {
              write(cpp_char("X"));
            } else
            {
              write(cpp_char("."));
            }
          }
        }
      } else
      {
        var num_even: dynamic = min(k, ((n) / 2));
        var num_odd: dynamic = ((k) - num_even);
        if (((x % 2) == 0))
        {
          var dist: dynamic = (((n - x)) / 2);
          if (((dist + 1) <= num_even))
          {
            write(cpp_char("X"));
          } else
          {
            write(cpp_char("."));
          }
        } else
        {
          var dist: dynamic = ((((n - 1) - x)) / 2);
          if (((dist + 1) <= num_odd))
          {
            write(cpp_char("X"));
          } else
          {
            write(cpp_char("."));
          }
        }
      }
    }
  }
  return 0;
}
