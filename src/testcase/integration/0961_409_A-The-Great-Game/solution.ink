// Translated from solution.cpp.

var a: dynamic;

var b: dynamic;

var wa: dynamic;

var wb: dynamic;

func main()
{
  read(a, b);
  {
    var i = 0;
    while ((i < a.size()))
    {
      if (((a[i] == cpp_char("8")) && (b[i] == cpp_char("["))))
      {
        wa += 1;
      }
      if (((a[i] == cpp_char("[")) && (b[i] == cpp_char("("))))
      {
        wa += 1;
      }
      if (((a[i] == cpp_char("(")) && (b[i] == cpp_char("8"))))
      {
        wa += 1;
      }
      if (((b[i] == cpp_char("8")) && (a[i] == cpp_char("["))))
      {
        wb += 1;
      }
      if (((b[i] == cpp_char("[")) && (a[i] == cpp_char("("))))
      {
        wb += 1;
      }
      if (((b[i] == cpp_char("(")) && (a[i] == cpp_char("8"))))
      {
        wb += 1;
      }
      i += 2;
    }
  }
  if ((wa > wb))
  {
    puts("TEAM 1 WINS");
  } else if ((wa < wb))
  {
    puts("TEAM 2 WINS");
  } else
  {
    puts("TIE");
  }
  return 0;
}
