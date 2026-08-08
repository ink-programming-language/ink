// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  var p: dynamic;
  read(n, k, p);
  while (cpp_update(p, "--"))
  {
    var x: dynamic;
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
          var num_even = min((k - 1), (((n - 1)) / 2));
          var num_odd = (((k - 1)) - num_even);
          if (((x % 2) == 0))
          {
            var dist = ((((n - 1) - x)) / 2);
            if (((dist + 1) <= num_even))
            {
              write(cpp_char("X"));
            } else
            {
              write(cpp_char("."));
            }
          } else
          {
            var dist = ((((n - 2) - x)) / 2);
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
        var num_even = min(k, ((n) / 2));
        var num_odd = ((k) - num_even);
        if (((x % 2) == 0))
        {
          var dist = (((n - x)) / 2);
          if (((dist + 1) <= num_even))
          {
            write(cpp_char("X"));
          } else
          {
            write(cpp_char("."));
          }
        } else
        {
          var dist = ((((n - 1) - x)) / 2);
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
