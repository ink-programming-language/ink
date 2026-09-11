// Translated from solution.cpp.

func read(x: dynamic) -> dynamic
{
  var f: dynamic = 1;
  x = 0;
  var c: dynamic = getchar();
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
  x *= f;
}

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(T);
  while (cpp_update(T, "--"))
  {
    read(n);
    read(m);
    {
      i = 1;
      while ((i <= n))
      {
        {
          j = 1;
          while ((j <= m))
          {
            read(x);
            printf("%d ",  (((x % 2) == (((i + j)) % 2))) ? x : (x + 1));
            j += 1;
          }
        }
        i += 1;
        putchar(cpp_char("\n"));
      }
    }
  }
  return 0;
}
