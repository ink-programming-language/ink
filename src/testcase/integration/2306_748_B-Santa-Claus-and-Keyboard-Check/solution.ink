// Translated from solution.cpp.

func main() -> dynamic
{
  var s1: dynamic = cpp_uninitialized();
  var s2: dynamic = cpp_uninitialized();
  read(s1, s2);
  var n: dynamic = s1.length();
  var m: dynamic = cpp_uninitialized();
  var arr: dynamic = [];
  var flag: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((s1[i] != s2[i]))
      {
        if ((arr[(s1[i] - cpp_char("a"))] == arr[(s2[i] - cpp_char("a"))]))
        {
          if ((arr[(s1[i] - cpp_char("a"))] == 0))
          {
            m.insert([s1[i], s2[i]]);
            arr[(s1[i] - cpp_char("a"))] = cpp_assign(arr[(s2[i] - cpp_char("a"))], "=", 1);
          } else
          {
            if (((m.find(s1[i])->second == s2[i]) || (m.find(s2[i])->second == s1[i])))
            {
              i += 1;
              continue;
            } else
            {
              flag = 0;
              break;
            }
          }
        } else
        {
          flag = 0;
          break;
        }
      } else
      {
        if (((arr[(s1[i] - cpp_char("a"))] == 0) || (arr[(s1[i] - cpp_char("a"))] == -1)))
        {
          arr[(s1[i] - cpp_char("a"))] = -1;
          i += 1;
          continue;
        } else
        {
          flag = 0;
          break;
        }
      }
      i += 1;
    }
  }
  if ((flag == 0))
  {
    write(-1);
    return 0;
  }
  write(m.size(), "\n");
  for (var a: dynamic in m)
  {
    write(a.first, " ", a.second, "\n");
  }
  return 0;
}
