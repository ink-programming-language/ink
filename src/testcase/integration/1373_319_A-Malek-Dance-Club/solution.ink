// Translated from solution.cpp.

var p = cpp_array(110);

var str = cpp_array(110);

func main()
{
  var i: dynamic;
  var n: dynamic;
  var res: dynamic;
  p[0] = 1;
  {
    i = 1;
    while ((i < 110))
    {
      p[i] = ((2 * p[(i - 1)]) % 1000000007);
      i += 1;
    }
  }
  scanf("%s ", str);
  n = strlen(str);
  res = 0;
  {
    i = 0;
    while ((i < n))
    {
      if ((str[i] == cpp_char("1")))
      {
        res = (((res + (((cpp_cast(p[i]) * p[((n - i) - 1)]) % 1000000007) * p[((n - i) - 1)]))) % 1000000007);
      }
      i += 1;
    }
  }
  printf("%d\n", res);
  return 0;
}
