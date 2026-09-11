// Translated from solution.cpp.

var N: dynamic = 200010;

var b: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var res: dynamic = -1;

var mp: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (b + i));
      mp[(b[i] - i)] += b[i];
      i += 1;
    }
  }
  {
    var it: dynamic = mp.begin();
    while ((it != mp.end()))
    {
      if ((res < it->second))
      {
        res = it->second;
      }
      it += 1;
    }
  }
  write(res, "\n");
  return 0;
}
