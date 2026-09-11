// Translated from solution.cpp.

var s: dynamic = cpp_array(110);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  read(n, k);
  getchar();
  {
    i = 0;
    while ((i < n))
    {
      read(s[i]);
      if ((s[i] == cpp_char("G")))
      {
        a = i;
      }
      if ((s[i] == cpp_char("T")))
      {
        b = i;
      }
      i += 1;
    }
  }
  if ((b > a))
  {
    {
      i = ((a + k));
      while ((i < n))
      {
        if ((s[i] == cpp_char("#")))
        {
          write("NO");
          return 0;
        }
        if ((i > b))
        {
          write("NO");
          return 0;
        }
        if ((s[i] == cpp_char("T")))
        {
          write("YES");
          return 0;
        }
        i += k;
      }
    }
    write("NO");
  } else
  {
    {
      i = a;
      while ((i >= 0))
      {
        if ((s[i] == cpp_char("#")))
        {
          write("NO");
          return 0;
        }
        if ((i < b))
        {
          write("NO");
          return 0;
        }
        if ((s[i] == cpp_char("T")))
        {
          write("YES");
          return 0;
        }
        i -= k;
      }
    }
    write("NO");
  }
  return 0;
}
