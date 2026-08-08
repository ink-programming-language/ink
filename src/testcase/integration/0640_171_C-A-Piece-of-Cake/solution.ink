// Translated from solution.cpp.

func getint()
{
  var f = 1;
  var x = 0;
  var c = getchar();
  while ((!isdigit(c)))
  {
    if ((c == cpp_char("-")))
    {
      f = -1;
    }
    c = getchar();
  }
  while (isdigit(c))
  {
    x = (((x * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  return (x * f);
}

func getll()
{
  var f = 1;
  var x = 0;
  var c = getchar();
  while ((!isdigit(c)))
  {
    if ((c == cpp_char("-")))
    {
      f = -1;
    }
    c = getchar();
  }
  while (isdigit(c))
  {
    x = (((x * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  return (x * f);
}

var a0: dynamic;

var ans: dynamic;

func main()
{
  a0 = getint();
  {
    int_cpp(i) = (1);
    while (((i) <= (a0)))
    {
      ans += ((i * getint()));
      (i) += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
