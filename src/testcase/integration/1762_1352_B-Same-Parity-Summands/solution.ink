// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var k: dynamic;
    var p = 0;
    var c = 0;
    var ck: dynamic;
    read(n, k);
    if ((k > n))
    {
      write("NO", "\n");
    } else
    {
      if (cpp_binary((cpp_binary(((k % 2) == 0), "and", ((n % 2) == 0))), "or", (cpp_binary(((n % 2) == 1), "and", ((k % 2) == 1)))))
      {
        if (((((n - ((k - 1)))) % 2) == 1))
        {
          write("YES", "\n");
          {
            var i = 0;
            while ((i < (k - 1)))
            {
              write(1, cpp_char(" "));
              i += 1;
            }
          }
          write((n - ((k - 1))), "\n");
        } else
        {
          write("NO", "\n");
        }
      } else if (cpp_binary(((n % 2) == 0), "and", (n >= (k * 2))))
      {
        write("YES", "\n");
        {
          var i = 0;
          while ((i < (k - 1)))
          {
            write(2, cpp_char(" "));
            i += 1;
          }
        }
        write((n - ((((k - 1)) * 2))), "\n");
      } else
      {
        write("NO", "\n");
      }
    }
  }
}
