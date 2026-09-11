// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0; i<(n); i++)");
}

var INF: dynamic = cpp_expression("#in");

func is_digit(x: dynamic) -> dynamic
{
  {
    var i: dynamic = 48;
    while ((i <= 57))
    {
      if ((x == i))
      {
        return true;
      }
      i += 1;
    }
  }
  return false;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var s: dynamic = cpp_uninitialized();
    var t: dynamic = cpp_uninitialized();
    read(s, t);
    {
      var i: dynamic = (s.size() - 1);
      while ((i >= 0))
      {
        if ((s[i] == cpp_char("J")))
        {
          rotate(t.rbegin(), (t.rbegin() + 1), t.rend());
        }
        if ((s[i] == cpp_char("C")))
        {
          rotate(t.begin(), (t.begin() + 1), t.end());
        }
        if ((s[i] == cpp_char("E")))
        {
          rep(j, (t.size() / 2));
          {
            swap(t[j], t[((t.size() - (t.size() / 2)) + j)]);
          }
        }
        if ((s[i] == cpp_char("A")))
        {
          reverse(t.begin(), t.end());
        }
        if ((s[i] == cpp_char("P")))
        {
          rep(j, t.size());
          {
            if (is_digit(t[j]))
            {
              if ((t[j] == 48))
              {
                t[j] = 57;
              } else
              {
                t[j] -= 1;
              }
            }
          }
        }
        if ((s[i] == cpp_char("M")))
        {
          rep(j, t.size());
          {
            if (is_digit(t[j]))
            {
              if ((t[j] == 57))
              {
                t[j] = 48;
              } else
              {
                t[j] += 1;
              }
            }
          }
        }
        i -= 1;
      }
    }
    write(t, "\n");
  }
