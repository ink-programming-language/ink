// Translated from solution.cpp.

func bs(a: dynamic, l1: dynamic, r1: dynamic, n: dynamic, m: dynamic)
{
  var l = l1;
  var r = r1;
  var mid: dynamic;
  while (((r - l) > 1))
  {
    mid = (l + (((r - l)) / 2));
    if (((((((mid / 2) + (mid / 3)) - (mid / 6)) >= a) && ((mid / 2) >= n)) && ((mid / 3) >= m)))
    {
      r = mid;
    } else
    {
      l = mid;
    }
  }
  if (((((((l / 2) + (l / 3)) - (l / 6)) >= a) && ((l / 2) >= n)) && ((l / 3) >= m)))
  {
    return l;
  } else
  {
    return r;
  }
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var ans = bs((n + m), 0, 1000000000, n, m);
  write(ans, "\n");
  return 0;
}
