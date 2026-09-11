// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var ara: dynamic = cpp_array(26);
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      ara[i] = 0;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    var len: dynamic = s.length();
    while ((i < len))
    {
      ara[(s[i] - cpp_char("a"))] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      if (((ara[i] % n) != 0))
      {
        printf("-1");
        return 0;
      }
      ara[i] /= n;
      i += 1;
    }
  }
  {
    var k: dynamic = 0;
    while ((k < n))
    {
      {
        var i: dynamic = 0;
        while ((i < 26))
        {
          {
            var j: dynamic = 0;
            while ((j < ara[i]))
            {
              printf("%c", (i + cpp_char("a")));
              j += 1;
            }
          }
          i += 1;
        }
      }
      k += 1;
    }
  }
  printf("\n");
}
