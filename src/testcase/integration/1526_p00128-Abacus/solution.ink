// Translated from solution.cpp.

func main()
{
  var c = 0;
  var n: dynamic;
  var str = cpp_array(5);
  while ((cin >> n))
  {
    if (c)
    {
      write("\n");
    }
    c = 1;
    {
      var i = 0;
      var k = 10000;
      while ((i < 5))
      {
        str[i] = (n / k);
        n %= k;
        i += 1;
        k /= 10;
      }
    }
    {
      var j = 0;
      while ((j < 2))
      {
        {
          var i = 0;
          while ((i < 5))
          {
            write((if (((str[i] / 5) == j)) cpp_char("*") else cpp_char(" ")));
            i += 1;
          }
        }
        write("\n");
        j += 1;
      }
    }
    {
      var i = 0;
      while ((i < 5))
      {
        write(cpp_char("="));
        i += 1;
      }
    }
    write("\n");
    {
      var j = 0;
      while ((j < 5))
      {
        {
          var i = 0;
          while ((i < 5))
          {
            write((if (((str[i] % 5) == j)) cpp_char(" ") else cpp_char("*")));
            i += 1;
          }
        }
        write("\n");
        j += 1;
      }
    }
  }
}
