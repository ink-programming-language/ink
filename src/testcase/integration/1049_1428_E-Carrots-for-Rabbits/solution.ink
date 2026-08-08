// Translated from solution.cpp.

class Split
{
  var totSz: dynamic;
  var pcs: dynamic;
}

func cost(s: dynamic)
{
  var q = (s.totSz / s.pcs);
  var r = (s.totSz % s.pcs);
  return ((((1 * q) * q) * ((s.pcs - r))) + (((1 * ((q + 1))) * ((q + 1))) * r));
}

func valNext(s: dynamic)
{
  var pc = cost(s);
  s.pcs += 1;
  assert(((pc - cost(s)) >= 0));
  return (pc - cost(s));
}

func nPieces(initLen: dynamic, val: dynamic)
{
  var L = 1;
  var R = initLen;
  while ((L < R))
  {
    var M = (((L + R)) / 2);
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

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var a = vector(n);
  var max_ai = 0;
  for (var ai in a)
  {
    read(ai);
    max_ai = max(max_ai, ai);
  }
  var L = 1;
  var R = ((1 * max_ai) * max_ai);
  while ((L < R))
  {
    var M = ((((L + R) + 1)) / 2);
    var pieceTot = 0;
    for (var ai in a)
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
  var pieceTot = 0;
  var sqTot = 0;
  for (var ai in a)
  {
    var ci = nPieces(ai, L);
    pieceTot += ci;
    sqTot += cost([ai, ci]);
  }
  assert((pieceTot >= k));
  sqTot += (L * ((pieceTot - k)));
  write(sqTot, cpp_char("\n"));
  return 0;
}
