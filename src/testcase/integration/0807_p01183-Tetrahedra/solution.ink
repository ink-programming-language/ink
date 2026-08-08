// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func tri(a: dynamic, b: dynamic, c: dynamic)
{
  var p = [a, b, c];
  sort(p, (p + 3));
  return ((p[0] + p[1]) > p[2]);
}

func volume(U: dynamic, V: dynamic, W: dynamic, u: dynamic, v: dynamic, w: dynamic)
{
  var X = ((((w - U) + v)) * (((U + v) + w)));
  var x = ((((U - v) + w)) * (((v - w) + U)));
  var Y = ((((u - V) + w)) * (((V + w) + u)));
  var y = ((((V - w) + u)) * (((w - u) + V)));
  var Z = ((((v - W) + u)) * (((W + u) + v)));
  var z = ((((W - u) + v)) * (((u - v) + W)));
  var a = sqrt(((x * Y) * Z));
  var b = sqrt(((y * Z) * X));
  var c = sqrt(((z * X) * Y));
  var d = sqrt(((x * y) * z));
  return (sqrt(((((((((-a) + b) + c) + d)) * ((((a - b) + c) + d))) * ((((a + b) - c) + d))) * ((((a + b) + c) - d)))) / ((((192 * u) * v) * w)));
}

func main()
{
  {
    var n: dynamic;
    while (cpp_comma(scanf("%d", (&n)), n))
    {
      var L = cpp_array(15);
      rep(i, n);
      scanf("%d", (L + i));
      var ans = 0;
      var p = [1, 1, 1, 1, 1, 1];
      while (true)
      {
        var m = 0;
        var q = cpp_array(6);
        rep(i, n);
        if ((p[i] == 1))
        {
          q[cpp_update(m, "++")] = i;
        }
        while (true)
        {
          var a = L[q[0]];
          var b = L[q[1]];
          var c = L[q[2]];
          var d = L[q[3]];
          var e = L[q[4]];
          var f = L[q[5]];
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
