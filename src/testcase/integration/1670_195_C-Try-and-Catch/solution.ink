// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var ar = cpp_array(n);
  var s: dynamic;
  var throw_line: dynamic;
  var thr: dynamic;
  var i = 0;
  var count = 0;
  var flag = 0;
  var count2 = 0;
  var v: dynamic;
  getline(cin, s);
  {
    var i = 0;
    while ((i < n))
    {
      getline(cin, s);
      if (((s.find("try") != -1) && (flag != 1)))
      {
        count += 1;
      } else if (((s.find("try") != -1) && (flag == 1)))
      {
        var k: dynamic;
        k = s.find(cpp_char("\""));
        var j: dynamic;
        j = s.find("try");
        if (((cpp_assign(j, "=", ((k + 1) && (k > 0)))) || (((k > j) && (k > 0)))))
        {
        } else
        {
          count2 += 1;
        }
      }
      if ((s.find("throw") != -1))
      {
        throw_line = s;
        flag = 1;
      }
      if ((s.find("catch") != -1))
      {
        if ((flag == 0))
        {
          count -= 1;
        } else
        {
          if ((count2 != 0))
          {
            count2 -= 1;
          } else
          {
            if ((count != 0))
            {
              count -= 1;
              v.push_back(s);
            }
          }
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < throw_line.size()))
    {
      if ((throw_line[i] == cpp_char("(")))
      {
        i += 1;
        while ((throw_line[i] != cpp_char(")")))
        {
          if ((throw_line[i] != cpp_char(" ")))
          {
            thr.push_back(throw_line[i]);
          }
          i += 1;
        }
      }
      i += 1;
    }
  }
  count2 = 0;
  {
    var i = 0;
    while ((i < v.size()))
    {
      s = v[i];
      if (((s.find(thr) != -1) && (count2 == 0)))
      {
        count2 += 1;
        {
          var i = 0;
          while ((i < s.size()))
          {
            if ((s[i] == cpp_char("\"")))
            {
              i += 1;
              while ((s[i] != cpp_char("\"")))
              {
                write(s[i]);
                i += 1;
              }
            }
            i += 1;
          }
        }
      }
      i += 1;
    }
  }
  if (count2)
  {
    write("\n");
  } else
  {
    write("Unhandled Exception", "\n");
  }
}
