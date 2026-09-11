// Translated from solution.cpp.

var pii: dynamic = cpp_expression("#include <bit");

var ff: dynamic = cpp_expression("#incl");

var ss: dynamic = cpp_expression("#inclu");

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    var s: dynamic = cpp_uninitialized();
    read(s);
    var T: dynamic = 0;
    var M: dynamic = 0;
    var t1: dynamic = cpp_uninitialized();
    var t2: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    for (var c: dynamic in s)
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
      var cnt: dynamic = 0;
      var i: dynamic = cpp_uninitialized();
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
      var chk: dynamic = 1;
      {
        var i: dynamic = 0;
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
