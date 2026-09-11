// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func tri(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  var p: dynamic = [a, b, c];
  sort(p, (p + 3));
  return ((p[0] + p[1]) > p[2]);
}

func volume(U: dynamic, V: dynamic, W: dynamic, u: dynamic, v: dynamic, w: dynamic) -> dynamic
{
  var X: dynamic = ((((w - U) + v)) * (((U + v) + w)));
  var x: dynamic = ((((U - v) + w)) * (((v - w) + U)));
  var Y: dynamic = ((((u - V) + w)) * (((V + w) + u)));
  var y: dynamic = ((((V - w) + u)) * (((w - u) + V)));
  var Z: dynamic = ((((v - W) + u)) * (((W + u) + v)));
  var z: dynamic = ((((W - u) + v)) * (((u - v) + W)));
  var a: dynamic = sqrt(((x * Y) * Z));
  var b: dynamic = sqrt(((y * Z) * X));
  var c: dynamic = sqrt(((z * X) * Y));
  var d: dynamic = sqrt(((x * y) * z));
  return (sqrt(((((((((-a) + b) + c) + d)) * ((((a - b) + c) + d))) * ((((a + b) - c) + d))) * ((((a + b) + c) - d)))) / ((((192 * u) * v) * w)));
}

func main() -> dynamic
{
  {
    var n: dynamic = cpp_uninitialized();
    while (cpp_comma(scanf("%d", (&n)), n))
    {
      var L: dynamic = cpp_array(15);
      rep(i, n);
      scanf("%d", (L + i));
      var ans: dynamic = 0;
      var p: dynamic = [1, 1, 1, 1, 1, 1];
      while (true)
      {
        var m: dynamic = 0;
        var q: dynamic = cpp_array(6);
        rep(i, n);
        if ((p[i] == 1))
        {
          q[cpp_update(m, "++")] = i;
        }
        while (true)
        {
          var a: dynamic = L[q[0]];
          var b: dynamic = L[q[1]];
          var c: dynamic = L[q[2]];
          var d: dynamic = L[q[3]];
          var e: dynamic = L[q[4]];
          var f: dynamic = L[q[5]];
          if ((((tri(a, b, c) && tri(a, e, f)) && tri(b, f, d)) && tri(c, d, e)))
          {
            ans = max(ans, volume(a, b, c, d, e, f));
          }
          if (!((next_permutation(q, (q + 5)))))
          {
            break;
          }
        }
        if (!((prev_permutation(p, (p + n)))))
        {
          break;
        }
      }
      printf("%.9f\n", ans);
    }
  }
  return 0;
}
