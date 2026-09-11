// Translated from solution.cpp.

var range: dynamic = ["-128", "127", "-32768", "32767", "-2147483648", "2147483647", "-9223372036854775808", "9223372036854775807"];

var a: dynamic = cpp_array(105);

func in_cpp(a: dynamic) -> dynamic
{
  var i: dynamic = 1;
  if ((a[0] == cpp_char("-")))
  {
    i -= 1;
  }
  var x: dynamic = strlen(a);
  {
    while ((i < 8))
    {
      var flag: dynamic = 0;
      if ((x < strlen(range[i])))
      {
        return i;
      }
      if ((x > strlen(range[i])))
      {
        i += 2;
        continue;
      }
      if ((x == strlen(range[i])))
      {
        var j: dynamic = cpp_uninitialized();
        {
          j = 0;
          while ((a[j] != cpp_char("\u{0}")))
          {
            if ((a[j] > range[i][j]))
            {
              flag = 1;
              break;
            }
            if ((a[j] < range[i][j]))
            {
              flag = 0;
              break;
            }
            j += 1;
          }
        }
        if ((flag == 0))
        {
          return i;
        }
      }
      i += 2;
    }
  }
  return 9;
}

func main() -> dynamic
{
  while ((scanf("%s", a) != EOF))
  {
    var x: dynamic = in_cpp(a);
    x /= 2;
    if ((x == 0))
    {
      printf("byte\n");
    }
    if ((x == 1))
    {
      printf("short\n");
    }
    if ((x == 2))
    {
      printf("int\n");
    }
    if ((x == 3))
    {
      printf("long\n");
    }
    if ((x == 4))
    {
      printf("BigInteger\n");
    }
  }
}
