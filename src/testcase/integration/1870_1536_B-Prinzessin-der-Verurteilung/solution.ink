// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  if ((a == 0))
  {
    return b;
  }
  return gcd((b % a), a);
}

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    var str: dynamic;
    read(str);
    var arr = [0];
    var c = 0;
    {
      var i = 0;
      while ((i < str.length()))
      {
        arr[(str[i] - 97)] += 1;
        if ((arr[(str[i] - 97)] == 1))
        {
          c += 1;
        }
        i += 1;
      }
    }
    var s = "";
    if ((c < 26))
    {
      {
        var i = 0;
        while ((i < 26))
        {
          if ((arr[i] == 0))
          {
            s = (s + cpp_cast(((i + 97))));
            break;
          }
          i += 1;
        }
      }
      write(s, "\n");
    } else
    {
      var z = cpp_char("a");
      {
        var ch = cpp_char("a");
        while ((ch <= cpp_char("z")))
        {
          {
            var k = cpp_char("a");
            while ((k <= cpp_char("z")))
            {
              var t = "";
              t += ch;
              t += k;
              var index: dynamic;
              if (((cpp_assign(index, "=", str.find(t, 0))) == string_cpp.npos))
              {
                write(t, "\n");
                ch = cpp_char("z");
                z = cpp_char("b");
                break;
              }
              k += 1;
            }
          }
          ch += 1;
        }
      }
      {
        while ((z < cpp_char("b")))
        {
          {
            var ch = cpp_char("a");
            while ((ch <= cpp_char("z")))
            {
              {
                var k = cpp_char("a");
                while ((k <= cpp_char("z")))
                {
                  var t = "";
                  t += z;
                  t += ch;
                  t += k;
                  var index: dynamic;
                  if (((cpp_assign(index, "=", str.find(t, 0))) == string_cpp.npos))
                  {
                    write(t, "\n");
                    ch = cpp_char("z");
                    z = cpp_char("b");
                    break;
                  }
                  k += 1;
                }
              }
              ch += 1;
            }
          }
          z += 1;
        }
      }
    }
  }
  return 0;
}
