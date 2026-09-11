// Translated from solution.cpp.

func rp(i: dynamic, s: dynamic, e: dynamic) -> dynamic
{
  cpp_macro("for(int i=(int)(s);i<(int)(e);++i)");
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var F: dynamic = cpp_construct(n, vi(10));
  rp(i, 0, n);
  rp(j, 0, 10);
  read(F[i][j]);
  var P: dynamic = cpp_construct(n, vi((10 + 1)));
  rp(i, 0, n);
  rp(j, 0, (10 + 1));
  read(P[i][j]);
  var ans: dynamic = (-10000000 * 100);
  rp(i, 1, (1 << 10));
  {
    var s: dynamic = 0;
    rp(j, 0, n);
    {
      var cj: dynamic = 0;
      rp(k, 0, 10);
      if ((((i >> k) & 1) && F[j][k]))
      {
        cj += 1;
      }
      s += P[j][cj];
    }
    ans = max(ans, s);
  }
  write(ans, "\n");
}
