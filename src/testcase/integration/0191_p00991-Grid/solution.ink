// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; i < int(n); ++i)");
}

var MOD: dynamic = 100000007;

var nck: dynamic = cpp_array(1111, 1111);

func main() -> dynamic
{
  rep(i, 1111)[i][0] = 1;
  rep(i, 1110)(j, 1110);
  {
    nck[(i + 1)][(j + 1)] = (((nck[i][j] + nck[i][(j + 1)])) % MOD);
  }
  var r: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var a1: dynamic = cpp_uninitialized();
  var a2: dynamic = cpp_uninitialized();
  var b1: dynamic = cpp_uninitialized();
  var b2: dynamic = cpp_uninitialized();
  var t: dynamic = 1;
  read(r, c, a1, a2, b1, b2);
  if ((r == (abs((a1 - b1)) * 2)))
  {
    t *= 2;
  }
  if ((c == (abs((a2 - b2)) * 2)))
  {
    t *= 2;
  }
  var aa: dynamic = min(abs((a1 - b1)), (r - abs((a1 - b1))));
  var bb: dynamic = min(abs((a2 - b2)), (c - abs((a2 - b2))));
  write(((nck[(aa + bb)][aa] * t) % MOD), "\n");
  return 0;
}
