// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((a == 0))
  {
    return b;
  }
  return gcd((b % a), a);
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    var str: dynamic = cpp_uninitialized();
    read(str);
    var arr: dynamic = [0];
    var c: dynamic = 0;
    {
      var i: dynamic = 0;
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
    var s: dynamic = "";
    if ((c < 26))
    {
      {
        var i: dynamic = 0;
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
      var z: dynamic = cpp_char("a");
      {
        var ch: dynamic = cpp_char("a");
        while ((ch <= cpp_char("z")))
        {
          {
            var k: dynamic = cpp_char("a");
            while ((k <= cpp_char("z")))
            {
              var t: dynamic = "";
              t += ch;
              t += k;
              var index: dynamic = cpp_uninitialized();
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
            var ch: dynamic = cpp_char("a");
            while ((ch <= cpp_char("z")))
            {
              {
                var k: dynamic = cpp_char("a");
                while ((k <= cpp_char("z")))
                {
                  var t: dynamic = "";
                  t += z;
                  t += ch;
                  t += k;
                  var index: dynamic = cpp_uninitialized();
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
