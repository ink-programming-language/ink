// Translated from solution.cpp.

var ll: dynamic = dynamic;

var N: dynamic = (1e5 + 10);

var d: dynamic = cpp_array(N);

var v: dynamic = cpp_array(N);

var Lm: dynamic = cpp_array(N);

var Rm: dynamic = cpp_array(N);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var C: dynamic = cpp_uninitialized();
  read(n, C);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld%lld", (&d[i]), (&v[i]));
      i += 1;
    }
  }
  var ans: dynamic = 0;
  var qz: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      qz += v[i];
      Lm[i] = max(Lm[(i - 1)], (qz - d[i]));
      i += 1;
    }
  }
  qz = 0;
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      qz += v[i];
      Rm[i] = max(Rm[(i + 1)], (qz - ((C - d[i]))));
      i -= 1;
    }
  }
  ans = max(ans, max(Lm[n], Rm[1]));
  qz = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      qz += v[i];
      ans = max(ans, ((qz - (d[i] * 2)) + Rm[(i + 1)]));
      i += 1;
    }
  }
  qz = 0;
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      qz += v[i];
      ans = max(ans, ((qz - (((C - d[i])) * 2)) + Lm[(i - 1)]));
      i -= 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
