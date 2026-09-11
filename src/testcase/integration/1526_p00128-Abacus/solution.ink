// Translated from solution.cpp.

func main() -> dynamic
{
  var c: dynamic = 0;
  var n: dynamic = cpp_uninitialized();
  var str: dynamic = cpp_array(5);
  while ((cin >> n))
  {
    if (c)
    {
      write("\n");
    }
    c = 1;
    {
      var i: dynamic = 0;
      var k: dynamic = 10000;
      while ((i < 5))
      {
        str[i] = (n / k);
        n %= k;
        i += 1;
        k /= 10;
      }
    }
    {
      var j: dynamic = 0;
      while ((j < 2))
      {
        {
          var i: dynamic = 0;
          while ((i < 5))
          {
            write(( (((str[i] / 5) == j)) ? cpp_char("*") : cpp_char(" ")));
            i += 1;
          }
        }
        write("\n");
        j += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < 5))
      {
        write(cpp_char("="));
        i += 1;
      }
    }
    write("\n");
    {
      var j: dynamic = 0;
      while ((j < 5))
      {
        {
          var i: dynamic = 0;
          while ((i < 5))
          {
            write(( (((str[i] % 5) == j)) ? cpp_char(" ") : cpp_char("*")));
            i += 1;
          }
        }
        write("\n");
        j += 1;
      }
    }
  }
}
