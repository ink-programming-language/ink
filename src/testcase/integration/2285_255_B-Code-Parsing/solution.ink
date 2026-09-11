// Translated from solution.cpp.

var s: dynamic = cpp_array(1000010);

var len: dynamic = cpp_uninitialized();

var cntx: dynamic = cpp_uninitialized();

var cnty: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  scanf("%s", s);
  len = strlen(s);
  {
    i = 0;
    cntx = 0;
    cnty = 0;
    while ((i < len))
    {
      if ((s[i] == cpp_char("x")))
      {
        cntx += 1;
      } else
      {
        cnty += 1;
      }
      i += 1;
    }
  }
  if ((cntx > cnty))
  {
    {
      i = 0;
      while ((i < (cntx - cnty)))
      {
        printf("x");
        i += 1;
      }
    }
    printf("\n");
  } else
  {
    {
      i = 0;
      while ((i < (cnty - cntx)))
      {
        printf("y");
        i += 1;
      }
    }
    printf("\n");
  }
  return 0;
}
