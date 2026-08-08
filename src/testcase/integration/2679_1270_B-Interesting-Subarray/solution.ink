// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  scanf("%d", (&t));
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    scanf("%d", (&n));
    var a = cpp_array(n);
    var ans = 0;
    {
      var i = 0;
      while ((i < n))
      {
        scanf("%d", (&a[i]));
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        if ((abs((a[i] - a[(i + 1)])) > 1))
        {
          puts("YES");
          printf("%d %d\n", (i + 1), (i + 2));
          ans = 1;
          break;
        }
        i += 1;
      }
    }
    if ((!ans))
    {
      puts("NO");
    }
  }
}
