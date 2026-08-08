// Translated from solution.cpp.

var range = ["-128", "127", "-32768", "32767", "-2147483648", "2147483647", "-9223372036854775808", "9223372036854775807"];

var a = cpp_array(105);

func in_cpp(a: dynamic)
{
  var i = 1;
  if ((a[0] == cpp_char("-")))
  {
    i -= 1;
  }
  var x = strlen(a);
  {
    while ((i < 8))
    {
      var flag = 0;
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
        var j: dynamic;
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

func main()
{
  while ((scanf("%s", a) != EOF))
  {
    var x = in_cpp(a);
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
