// Translated from solution.cpp.

func read(x: dynamic)
{
  var f = 1;
  x = 0;
  var c = getchar();
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

func main()
{
  var T: dynamic;
  var x: dynamic;
  var i: dynamic;
  var j: dynamic;
  var n: dynamic;
  var m: dynamic;
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
            printf("%d ", if (((x % 2) == (((i + j)) % 2))) x else (x + 1));
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
