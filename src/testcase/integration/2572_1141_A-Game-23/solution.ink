// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  if (((m % n) != 0))
  {
    write("-1", "\n");
    return 0;
  }
  var d = (m / n);
  var ans = 0;
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
