// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  var x: dynamic;
  read(N, x);
  {
    var i = 0;
    while ((i < N))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a.begin(), a.end());
  var n: dynamic;
  {
    n = 0;
    while (((n < N) && (x > 0)))
    {
      x -= a[n];
      n += 1;
    }
  }
  if ((x != 0))
  {
    n -= 1;
  }
  write(n, "\n");
  return 0;
}
