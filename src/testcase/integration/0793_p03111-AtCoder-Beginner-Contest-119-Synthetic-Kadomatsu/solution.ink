// Translated from solution.cpp.

var INF = 10000000;

var N: dynamic;

var A: dynamic;

var B: dynamic;

var C: dynamic;

var L: dynamic;

func rec(i: dynamic, a: dynamic, b: dynamic, c: dynamic)
{
  if ((i == N))
  {
    if ((((!a) || (!b)) || (!c)))
    {
      return INF;
    }
    return ((abs((a - A)) + abs((b - B))) + abs((c - C)));
  }
  var res = rec((i + 1), a, b, c);
  res = min(res, (rec((i + 1), (a + L[i]), b, c) + (if (a) 10 else 0)));
  res = min(res, (rec((i + 1), a, (b + L[i]), c) + (if (b) 10 else 0)));
  res = min(res, (rec((i + 1), a, b, (c + L[i])) + (if (c) 10 else 0)));
  return res;
}

func main(argument_0: dynamic)
{
  read(N, A, B, C);
  L.resize(N);
  {
    var i = 0;
    while ((i < N))
    {
      read(L[i]);
      i += 1;
    }
  }
  write(rec(0, 0, 0, 0), "\n");
}
