// Translated from solution.cpp.

var INF: dynamic = 10000000;

var N: dynamic = cpp_uninitialized();

var A: dynamic = cpp_uninitialized();

var B: dynamic = cpp_uninitialized();

var C: dynamic = cpp_uninitialized();

var L: dynamic = cpp_uninitialized();

func rec(i: dynamic, a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  if ((i == N))
  {
    if ((((!a) || (!b)) || (!c)))
    {
      return INF;
    }
    return ((abs((a - A)) + abs((b - B))) + abs((c - C)));
  }
  var res: dynamic = rec((i + 1), a, b, c);
  res = min(res, (rec((i + 1), (a + L[i]), b, c) + ( (a) ? 10 : 0)));
  res = min(res, (rec((i + 1), a, (b + L[i]), c) + ( (b) ? 10 : 0)));
  res = min(res, (rec((i + 1), a, b, (c + L[i])) + ( (c) ? 10 : 0)));
  return res;
}

func main(argument_0: dynamic) -> dynamic
{
  read(N, A, B, C);
  L.resize(N);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(L[i]);
      i += 1;
    }
  }
  write(rec(0, 0, 0, 0), "\n");
}
