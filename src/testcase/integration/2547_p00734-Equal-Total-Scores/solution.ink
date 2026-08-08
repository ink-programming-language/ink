// Translated from solution.cpp.

var s = cpp_array(201);

func main()
{
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  while (1)
  {
    read(n, m);
    if (((n == 0) && (m == 0)))
    {
      break;
    }
    var sum_T: dynamic;
    var sum_H: dynamic;
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
