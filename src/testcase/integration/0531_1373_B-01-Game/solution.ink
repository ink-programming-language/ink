// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  var zer: dynamic = cpp_uninitialized();
  var jed: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(t);
  var s: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < t))
    {
      read(s);
      zer = 0;
      jed = 0;
      {
        var j: dynamic = 0;
        while ((j < s.size()))
        {
          var __cpp_switch_1: dynamic = s[j];
          if (__cpp_switch_1 == cpp_char("0"))
          {
            zer += 1;
            break;
          }
          else if (__cpp_switch_1 == cpp_char("1"))
          {
            jed += 1;
            break;
          }
          j += 1;
        }
      }
      if ((zer <= jed))
      {
        x = zer;
      } else
      {
        x = jed;
      }
      if (((x % 2) == 1))
      {
        write("DA", "\n");
      } else
      {
        write("NET", "\n");
      }
      i += 1;
    }
  }
}
