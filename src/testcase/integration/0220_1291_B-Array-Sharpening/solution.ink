// Translated from solution.cpp.

func read()
{
  var c = getchar();
  var x = 0;
  var f = 1;
  while (((c < cpp_char("0")) || (c > cpp_char("9"))))
  {
    if ((c == cpp_char("-")))
    {
      f = -1;
    }
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = (((x * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  return (x * f);
}

var inf = (2147483647 - 1);

var maxn = (3e5 + 10);

var a = cpp_array(maxn);

func main()
{
  var T: dynamic;
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    var n = read();
    {
      var i = 1;
      while ((i <= n))
      {
        a[i] = read();
        i += 1;
      }
    }
    var l: dynamic;
    var r: dynamic;
    {
      var i = 1;
      while ((i <= n))
      {
        if ((a[i] >= (i - 1)))
        {
          l = i;
        } else
        {
          break;
        }
        i += 1;
      }
    }
    {
      var i = n;
      while (i)
      {
        if ((a[i] >= (n - i)))
        {
          r = i;
        } else
        {
          break;
        }
        i -= 1;
      }
    }
    if ((l >= r))
    {
      puts("Yes");
    } else
    {
      puts("No");
    }
  }
  return 0;
}
