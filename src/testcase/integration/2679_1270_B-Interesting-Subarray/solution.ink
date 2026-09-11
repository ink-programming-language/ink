// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  scanf("%d", (&t));
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    scanf("%d", (&n));
    var a: dynamic = cpp_array(n);
    var ans: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        scanf("%d", (&a[i]));
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
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
