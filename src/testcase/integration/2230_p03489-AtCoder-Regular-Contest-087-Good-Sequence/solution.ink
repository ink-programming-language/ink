// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%lld", (&N));
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      scanf("%lld", (&a));
      m[a] += 1;
      i += 1;
    }
  }
  a = 0;
  for (var i: dynamic in m)
  {
    a +=  ((i.second >= i.first)) ? (i.second - i.first) : i.second;
  }
  write(a, "\n");
  return 0;
}
