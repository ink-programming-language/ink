// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  if (((m % n) != 0))
  {
    write("-1", "\n");
    return 0;
  }
  var d: dynamic = (m / n);
  var ans: dynamic = 0;
  while (((d % 2) == 0))
  {
    d = (d / 2);
    ans += 1;
  }
  while (((d % 3) == 0))
  {
    d = (d / 3);
    ans += 1;
  }
  if ((d == 1))
  {
    write(ans, "\n");
  } else
  {
    write("-1", "\n");
  }
}
