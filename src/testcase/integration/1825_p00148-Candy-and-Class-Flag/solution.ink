// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while ((cin >> n))
  {
    if ((!((n % 39))))
    {
      puts("3C39");
    } else
    {
      printf("3C%02d\n", (n % 39));
    }
  }
}
