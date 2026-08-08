// Translated from solution.cpp.

func main()
{
  var r1: dynamic;
  var r2: dynamic;
  var r3: dynamic;
  var str: dynamic;
  var fm: dynamic;
  var f: dynamic;
  while ((cin >> fm))
  {
    r1 = cpp_assign(r2, "=", 0);
    r3 = cpp_char(" ");
    f = true;
    str = "";
    {
      var i = 0;
      while ((f && (i < fm.size())))
      {
        if (isdigit(fm[i]))
        {
          str += fm[i];
        } else
        {
          r2 = atoi(str.c_str());
          str = "";
          if ((r3 == cpp_char(" ")))
          {
            r1 = r2;
          } else if ((r3 == cpp_char("+")))
          {
            r1 += r2;
          } else if ((r3 == cpp_char("-")))
          {
            r1 -= r2;
          } else if ((r3 == cpp_char("*")))
          {
            r1 *= r2;
          } else if ((r3 == cpp_char("=")))
          {
            break;
          }
          if (((((r1 < 0) || (r2 < 0)) || (r2 >= 10000)) || (r1 >= 10000)))
          {
            f = false;
          }
          r3 = fm[i];
        }
        i += 1;
      }
    }
    if ((!f))
    {
      write("E", "\n");
    } else
    {
      write(r1, "\n");
    }
  }
  return 0;
}
