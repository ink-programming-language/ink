// Translated from solution.cpp.

var INF: dynamic = 0x3f3f3f3f;

func in_cpp() -> dynamic
{
  var x: dynamic = 0;
  var c: dynamic = cpp_uninitialized();
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

var k: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(k);
  {
    var i: dynamic = 0;
    while ((i < ((k - 1))))
    {
      {
        var j: dynamic = 0;
        while ((j < ((k - 1))))
        {
          var pro: dynamic = (((i + 1)) * ((j + 1)));
          if ((pro >= k))
          {
            var tmp: dynamic = (pro / k);
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
