// Translated from solution.cpp.

func main()
{
  var t = 0;
  var sl = 0;
  var s = cpp_array(100010);
  read(s);
  {
    var i = 0;
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
    var t1 = 0;
    var t2 = 0;
    {
      var i = 0;
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
      var i = 1;
      while ((i < t))
      {
        write(cpp_char("1"), "\n");
        i += 1;
      }
    }
    write(((sl - t) + 1));
  }
}
