// Translated from solution.cpp.

func read() -> dynamic
{
  var c: dynamic = getchar();
  var x: dynamic = 0;
  var f: dynamic = 1;
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

var inf: dynamic = (2147483647 - 1);

var maxn: dynamic = (3e5 + 10);

var a: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    var n: dynamic = read();
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        a[i] = read();
        i += 1;
      }
    }
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 1;
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
      var i: dynamic = n;
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
