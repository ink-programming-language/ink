// Translated from solution.cpp.

var s = cpp_array(110);

func main()
{
  var n: dynamic;
  var k: dynamic;
  var a: dynamic;
  var b: dynamic;
  var i: dynamic;
  var j: dynamic;
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
