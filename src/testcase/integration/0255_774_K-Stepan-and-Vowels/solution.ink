// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  var Count: dynamic;
  var Number: dynamic;
  var E = cpp_array(5);
  scanf("%d", (&Number));
  gets(E);
  var String = cpp_array((Number + 1));
  gets(String);
  {
    Count = 0;
    while ((Count < Number))
    {
      if (((((((String[Count] != cpp_char("a")) && (String[Count] != cpp_char("e"))) && (String[Count] != cpp_char("i"))) && (String[Count] != cpp_char("o"))) && (String[Count] != cpp_char("u"))) && (String[Count] != cpp_char("y"))))
      {
        printf("%c", String[Count]);
      } else
      {
        if ((String[Count] != String[(Count - 1)]))
        {
          printf("%c", String[Count]);
        }
        if (((((((String[Count] == cpp_char("e")) || (String[Count] == cpp_char("o")))) && (String[(Count - 1)] == String[Count])) && (String[(Count - 2)] != String[Count])) && (String[(Count + 1)] != String[Count])))
        {
          printf("%c", String[Count]);
        }
      }
      Count += 1;
    }
  }
  printf("\n");
  return 0;
}
