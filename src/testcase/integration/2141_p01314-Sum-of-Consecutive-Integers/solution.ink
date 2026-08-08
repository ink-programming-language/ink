// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  while ((n != 0))
  {
    var s = 1;
    var sum = 0;
    var e = 1;
    var o = 0;
    while ((e <= n))
    {
      if ((sum < n))
      {
        sum += e;
        e += 1;
      } else if ((sum == n))
      {
        o += 1;
        sum += e;
        e += 1;
      } else
      {
        sum -= s;
        s += 1;
      }
    }
    write(o, "\n");
    read(n);
  }
  return 0;
}
