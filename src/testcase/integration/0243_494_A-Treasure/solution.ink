// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = 0;
  var sl: dynamic = 0;
  var s: dynamic = cpp_array(100010);
  read(s);
  {
    var i: dynamic = 0;
    while ((i < strlen(s)))
    {
      if ((s[i] == cpp_char("#")))
      {
        t += 1;
      } else if ((s[i] == cpp_char("(")))
      {
        sl += 1;
      } else
      {
        sl -= 1;
      }
      i += 1;
    }
  }
  if ((sl <= 0))
  {
    write("-1");
  } else
  {
    var t1: dynamic = 0;
    var t2: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < strlen(s)))
      {
        if ((s[i] == cpp_char("#")))
        {
          t1 += 1;
          if ((t1 != t))
          {
            t2 -= 1;
          } else
          {
            t2 = (((t2 - sl) + t) - 1);
          }
        } else if ((s[i] == cpp_char("(")))
        {
          t2 += 1;
        } else
        {
          t2 -= 1;
        }
        if ((t2 < 0))
        {
          write("-1");
          return 0;
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i < t))
      {
        write(cpp_char("1"), "\n");
        i += 1;
      }
    }
    write(((sl - t) + 1));
  }
}
