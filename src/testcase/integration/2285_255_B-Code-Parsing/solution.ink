// Translated from solution.cpp.

var s = cpp_array(1000010);

var len: dynamic;

var cntx: dynamic;

var cnty: dynamic;

func main()
{
  var i: dynamic;
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
