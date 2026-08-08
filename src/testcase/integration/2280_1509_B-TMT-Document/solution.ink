// Translated from solution.cpp.

var pii = cpp_expression("#include <bit");

var ff = cpp_expression("#incl");

var ss = cpp_expression("#inclu");

func main()
{
  ios.sync_with_stdio(false);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    var s: dynamic;
    read(s);
    var T = 0;
    var M = 0;
    var t1: dynamic;
    var t2: dynamic;
    var m: dynamic;
    for (var c in s)
    {
      if ((c == cpp_char("T")))
      {
        T += 1;
      } else
      {
        M += 1;
      }
    }
    if (((s[0] == cpp_char("M")) || (s[(s.length() - 1)] == cpp_char("M"))))
    {
      write("NO\n");
      continue;
    }
    if (((!((T & 1))) && ((2 * M) == T)))
    {
      var cnt = 0;
      var i: dynamic;
      {
        i = 0;
        while (((i < n) && (cnt < M)))
        {
          if ((s[i] == cpp_char("T")))
          {
            t1.push_back(i);
            cnt += 1;
          }
          i += 1;
        }
      }
      cnt = 0;
      {
        while (((i < n) && (cnt < M)))
        {
          if ((s[i] == cpp_char("T")))
          {
            t2.push_back(i);
            cnt += 1;
          }
          i += 1;
        }
      }
      cnt = 0;
      {
        i = 0;
        while (((i < n) && (cnt < M)))
        {
          if ((s[i] == cpp_char("M")))
          {
            m.push_back(i);
            cnt += 1;
          }
          i += 1;
        }
      }
      var chk = 1;
      {
        var i = 0;
        while (((i < m.size()) && chk))
        {
          if (((t1[i] < m[i]) && (m[i] < t2[i])))
          {
          } else
          {
            chk = 0;
          }
          i += 1;
        }
      }
      if (chk)
      {
        write("YES\n");
      } else
      {
        write("NO\n");
      }
    } else
    {
      write("NO\n");
    }
  }
}
