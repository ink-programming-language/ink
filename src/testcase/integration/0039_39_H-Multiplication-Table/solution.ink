// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

func in_cpp()
{
  var x = 0;
  var c: dynamic;
  {
    while ((cpp_cast((((cpp_assign(c, "=", getchar())) - cpp_char("0")))) >= 10))
    {
      if ((c == cpp_char("-")))
      {
        return (-in_cpp());
      }
      if ((!(~c)))
      {
        return (~0);
      }
    }
  }
  while (true)
  {
    x = ((((x << 3)) + ((x << 1))) + ((c - cpp_char("0"))));
    if (!(((cpp_cast((((cpp_assign(c, "=", getchar())) - cpp_char("0")))) < 10))))
    {
      break;
    }
  }
  return x;
}

var k: dynamic;

func main()
{
  read(k);
  {
    var i = 0;
    while ((i < ((k - 1))))
    {
      {
        var j = 0;
        while ((j < ((k - 1))))
        {
          var pro = (((i + 1)) * ((j + 1)));
          if ((pro >= k))
          {
            var tmp = (pro / k);
            pro %= k;
            pro += (tmp * 10);
          }
          write(pro, cpp_char(" "));
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
  return 0;
}
