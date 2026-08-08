// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  var zer: dynamic;
  var jed: dynamic;
  var x: dynamic;
  read(t);
  var s: dynamic;
  {
    var i = 0;
    while ((i < t))
    {
      read(s);
      zer = 0;
      jed = 0;
      {
        var j = 0;
        while ((j < s.size()))
        {
          var __cpp_switch_1 = s[j];
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
