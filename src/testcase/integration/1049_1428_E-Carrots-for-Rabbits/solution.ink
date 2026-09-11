// Translated from solution.cpp.

class Split
{
  var totSz: dynamic = cpp_uninitialized();
  var pcs: dynamic = cpp_uninitialized();
}

func cost(s: dynamic) -> dynamic
{
  var q: dynamic = (s.totSz / s.pcs);
  var r: dynamic = (s.totSz % s.pcs);
  return ((((1 * q) * q) * ((s.pcs - r))) + (((1 * ((q + 1))) * ((q + 1))) * r));
}

func valNext(s: dynamic) -> dynamic
{
  var pc: dynamic = cost(s);
  s.pcs += 1;
  assert(((pc - cost(s)) >= 0));
  return (pc - cost(s));
}

func nPieces(initLen: dynamic, val: dynamic) -> dynamic
{
  var L: dynamic = 1;
  var R: dynamic = initLen;
  while ((L < R))
  {
    var M: dynamic = (((L + R)) / 2);
    if ((valNext([initLen, M]) < val))
    {
      R = M;
    } else
    {
      L = (M + 1);
    }
  }
  return L;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var a: dynamic = vector(n);
  var max_ai: dynamic = 0;
  for (var ai: dynamic in a)
  {
    read(ai);
    max_ai = max(max_ai, ai);
  }
  var L: dynamic = 1;
  var R: dynamic = ((1 * max_ai) * max_ai);
  while ((L < R))
  {
    var M: dynamic = ((((L + R) + 1)) / 2);
    var pieceTot: dynamic = 0;
    for (var ai: dynamic in a)
    {
      pieceTot += nPieces(ai, M);
    }
    if ((pieceTot < k))
    {
      R = (M - 1);
    } else
    {
      L = M;
    }
  }
  var pieceTot: dynamic = 0;
  var sqTot: dynamic = 0;
  for (var ai: dynamic in a)
  {
    var ci: dynamic = nPieces(ai, L);
    pieceTot += ci;
    sqTot += cost([ai, ci]);
  }
  assert((pieceTot >= k));
  sqTot += (L * ((pieceTot - k)));
  write(sqTot, cpp_char("\n"));
  return 0;
}
