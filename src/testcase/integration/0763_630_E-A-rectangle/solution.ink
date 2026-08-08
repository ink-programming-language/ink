// Translated from solution.cpp.

func IN()
{
  var x = 0;
  var ch = getchar();
  var f = 1;
  while ((((!isdigit(ch)) && ((ch != cpp_char("-")))) && ((ch != EOF))))
  {
    ch = getchar();
  }
  if ((ch == cpp_char("-")))
  {
    f = -1;
    ch = getchar();
  }
  while (isdigit(ch))
  {
    x = (((((x << 1)) + ((x << 3))) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * f);
}

var X1: dynamic;

var Y1: dynamic;

var X2: dynamic;

var Y2: dynamic;

var dh: dynamic;

var sh: dynamic;

func main()
{
  X1 = IN();
  Y1 = IN();
  X2 = IN();
  Y2 = IN();
  dh = (((((Y2 - Y1) + 1)) / 2) + 1);
  sh = (((Y2 - Y1)) / 2);
  printf("%I64d\n", (((1 * dh) * (((((X2 - X1)) / 2) + 1))) + (((1 * sh) * ((X2 - X1))) / 2)));
}
