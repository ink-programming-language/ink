// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  while ((~scanf("%d", (&n))))
  {
    printf("%d", n);
    {
      var i = 1;
      while ((i < n))
      {
        printf(" %d", i);
        i += 1;
      }
    }
    printf("\n");
  }
  return 0;
}
