// Translated from solution.cpp.

func main() -> dynamic
{
  var r1: dynamic = cpp_uninitialized();
  var r2: dynamic = cpp_uninitialized();
  var r3: dynamic = cpp_uninitialized();
  var str: dynamic = cpp_uninitialized();
  var fm: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  while ((cin >> fm))
  {
    r1 = cpp_assign(r2, "=", 0);
    r3 = cpp_char(" ");
    f = true;
    str = "";
    {
      var i: dynamic = 0;
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
