// Translated from solution.cpp.

var n: dynamic;

var cnt = cpp_array(30);

var S = cpp_array(5);

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%s", S);
      cnt[(S[0] - 65)] += 1;
      i += 1;
    }
  }
  printf("AC x %d\n", cnt[0]);
  printf("WA x %d\n", cnt[(cpp_char("W") - 65)]);
  printf("TLE x %d\n", cnt[(cpp_char("T") - 65)]);
  printf("RE x %d\n", cnt[(cpp_char("R") - 65)]);
  return 0;
}
