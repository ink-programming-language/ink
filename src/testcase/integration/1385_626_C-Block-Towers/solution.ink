// Translated from solution.cpp.

func bs(a: dynamic, l1: dynamic, r1: dynamic, n: dynamic, m: dynamic) -> dynamic
{
  var l: dynamic = l1;
  var r: dynamic = r1;
  var mid: dynamic = cpp_uninitialized();
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

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var ans: dynamic = bs((n + m), 0, 1000000000, n, m);
  write(ans, "\n");
  return 0;
}
