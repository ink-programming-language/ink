// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    var p: dynamic = 0;
    var c: dynamic = 0;
    var ck: dynamic = cpp_uninitialized();
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
            var i: dynamic = 0;
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
          var i: dynamic = 0;
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
