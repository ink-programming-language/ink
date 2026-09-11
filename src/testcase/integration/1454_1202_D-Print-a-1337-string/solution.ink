// Translated from solution.cpp.

func rd() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * f);
}

var maxn: dynamic = 100010;

var t: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    var y: dynamic = 1;
    {
      while ((((y * ((y - 1))) / 2) <= n))
      {
        y += 1;
      }
    }
    y -= 1;
    n -= ((y * ((y - 1))) / 2);
    write("133");
    y -= 2;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        printf("7");
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= y))
      {
        printf("3");
        i += 1;
      }
    }
    write("7", "\n");
  }
}
