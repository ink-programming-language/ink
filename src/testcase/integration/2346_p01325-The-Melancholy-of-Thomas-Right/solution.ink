// Translated from solution.cpp.

var n: dynamic;

var t = cpp_array(10001);

var s = cpp_array(10001);

func main(argument_0: dynamic)
{
  while (cpp_comma((cin >> n), n))
  {
    {
      var i = 0;
      while ((i < n))
      {
        read(t[i]);
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        read(s[i]);
        i += 1;
      }
    }
    sort(s, (s + n));
    reverse(s, (s + n));
    var flg = true;
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = 0;
          while ((j < n))
          {
            if ((t[j] > 0))
            {
              t[j] -= 1;
              s[i] -= 1;
            }
            j += 1;
          }
        }
        if ((s[i] != 0))
        {
          flg = false;
          break;
        }
        i += 1;
      }
    }
    write((if (flg) "Yes" else "No"), "\n");
  }
  return 0;
}
