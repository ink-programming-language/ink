// Translated from solution.cpp.

var m: dynamic;

var s = cpp_array(120000);

var n: dynamic;

var us = cpp_array(120000);

func main()
{
  scanf("%d", (&m));
  scanf(" %s", s);
  n = strlen(s);
  {
    var i = 0;
    while ((i < 26))
    {
      {
        var j = 0;
        while ((j < n))
        {
          if ((s[j] == (cpp_char("a") + i)))
          {
            us[j] = 2;
          }
          j += 1;
        }
      }
      var pr = -1;
      var lst = ((-m) - 100);
      var fl = 0;
      {
        var j = 0;
        while ((j < n))
        {
          if ((us[j] == 1))
          {
            if (((j - pr) <= m))
            {
              pr = j;
              j += 1;
              continue;
            }
            if (((j - lst) <= m))
            {
              us[lst] = 1;
              pr = j;
            } else
            {
              fl = 1;
              break;
            }
          } else if ((us[j] == 2))
          {
            if (((j - pr) <= m))
            {
              lst = j;
            } else if (((j - lst) <= m))
            {
              us[lst] = 1;
              pr = lst;
              lst = j;
            } else
            {
              fl = 1;
              break;
            }
          }
          j += 1;
        }
      }
      if (((!fl) && ((pr + m) < n)))
      {
        if (((lst + m) >= n))
        {
          us[lst] = 1;
        } else
        {
          fl = 1;
        }
      }
      if (fl)
      {
        {
          var j = 0;
          while ((j < n))
          {
            if (us[j])
            {
              us[j] = 1;
            }
            j += 1;
          }
        }
      } else
      {
        var ans: dynamic;
        {
          var i = 0;
          while ((i < n))
          {
            if ((us[i] == 1))
            {
              ans += s[i];
            }
            i += 1;
          }
        }
        sort(ans.begin(), ans.end());
        write(ans, "\n");
        return 0;
      }
      i += 1;
    }
  }
  return 0;
}
