// Translated from solution.cpp.

var s: dynamic = cpp_array(201);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  while (1)
  {
    read(n, m);
    if (((n == 0) && (m == 0)))
    {
      break;
    }
    var sum_T: dynamic = cpp_uninitialized();
    var sum_H: dynamic = cpp_uninitialized();
    sum_T = cpp_assign(sum_H, "=", 0);
    {
      i = 0;
      while ((i < n))
      {
        read(s[i]);
        sum_T += s[i];
        i += 1;
      }
    }
    {
      i = n;
      while ((i < (n + m)))
      {
        read(s[i]);
        sum_H += s[i];
        i += 1;
      }
    }
    {
      i = 0;
      while ((i < n))
      {
        {
          j = n;
          while ((j < (n + m)))
          {
            if ((((sum_T - s[i]) + s[j]) == ((sum_H + s[i]) - s[j])))
            {
              write(s[i], " ", s[j], "\n");
              cpp_goto("goto loop;");
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    write(-1, "\n");
  }
  return 0;
}
