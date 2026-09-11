// Translated from solution.cpp.

var s: dynamic = cpp_array(3, 101);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var k: dynamic = 0;
  var c1: dynamic = 0;
  var c2: dynamic = 0;
  cin.get(s[0], 3);
  read(n);
  {
    i = 1;
    while ((i <= n))
    {
      cin.get();
      cin.get(s[i], 3);
      i += 1;
    }
  }
  {
    i = 1;
    while (((i <= n) && (k < 2)))
    {
      if (((s[0][0] == s[i][1]) && (c1 == 0)))
      {
        k += 1;
        c1 = 1;
      }
      if (((s[0][1] == s[i][0]) && (c2 == 0)))
      {
        k += 1;
        c2 = 1;
      }
      if ((strcmp(s[0], s[i]) == null))
      {
        k = 2;
      }
      i += 1;
    }
  }
  if ((k == 2))
  {
    write("YES");
  } else
  {
    write("NO");
  }
  return 0;
}
