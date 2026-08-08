// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    var s: dynamic;
    read(s);
    var v = cpp_array(10);
    {
      var i = 0;
      while ((i < n))
      {
        var x = (s[i] - cpp_char("0"));
        v[x].push_back(i);
        i += 1;
      }
    }
    var ans = cpp_array(n);
    var mn1 = -1;
    var mn2 = -1;
    var m1 = 0;
    var m2 = 0;
    var ok1 = true;
    var ok = true;
    {
      var i = 0;
      while ((i < 10))
      {
        if ((v[i].size() != 0))
        {
          for (var x in v[i])
          {
            if (((x > mn1) && ok1))
            {
              ans[x] = 1;
              mn1 = max(mn1, x);
              m1 = max(m1, i);
            } else if (((x > mn1) && (i <= m2)))
            {
              ans[x] = 1;
              mn1 = max(mn1, x);
              m1 = max(m1, i);
            } else if ((x > mn2))
            {
              ok1 = false;
              ans[x] = 2;
              mn2 = max(mn2, x);
              if ((m2 == 0))
              {
                m2 = i;
              }
            } else
            {
              ok = false;
              break;
            }
          }
          if ((!ok))
          {
            break;
          }
        }
        i += 1;
      }
    }
    if ((!ok))
    {
      write("-", cpp_char("\n"));
    } else
    {
      {
        var i = 0;
        while ((i < n))
        {
          write(ans[i]);
          i += 1;
        }
      }
      write(cpp_char("\n"));
    }
  }
}
