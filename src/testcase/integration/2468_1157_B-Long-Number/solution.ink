// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var i: dynamic;
  read(n);
  var s: dynamic;
  read(s);
  var a = cpp_array(n);
  var f = cpp_array(9);
  {
    i = 0;
    while ((i < n))
    {
      a[i] = cpp_cast(((s[i] - cpp_char("0"))));
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < 9))
    {
      read(f[i]);
      i += 1;
    }
  }
  var j = 0;
  var k = 0;
  var flag = 0;
  {
    i = 0;
    while ((i < n))
    {
      if ((f[(a[i] - 1)] >= a[i]))
      {
        if ((f[(a[i] - 1)] > a[i]))
        {
          flag = 1;
        }
        a[i] = f[(a[i] - 1)];
      } else if ((flag == 1))
      {
        break;
      }
      i += 1;
    }
  }
  var str = "";
  {
    i = 0;
    while ((i < n))
    {
      str += cpp_cast(((cpp_char("0") + a[i])));
      i += 1;
    }
  }
  write(str, "\n");
}
