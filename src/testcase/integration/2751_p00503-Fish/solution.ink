// Translated from solution.cpp.

var MAX_N = cpp_expression("#i");

var N: dynamic;

var K: dynamic;

var X1 = cpp_array(MAX_N);

var Y1 = cpp_array(MAX_N);

var Z1 = cpp_array(MAX_N);

var X2 = cpp_array(MAX_N);

var Y2 = cpp_array(MAX_N);

var Z2 = cpp_array(MAX_N);

var xsLen: dynamic;

var ysLen: dynamic;

var zsLen: dynamic;

var xs = cpp_array((MAX_N * 2));

var ys = cpp_array((MAX_N * 2));

var zs = cpp_array((MAX_N * 2));

func main()
{
  var i: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  scanf("%d%d", (&N), (&K));
  {
    i = 0;
    while ((i < N))
    {
      scanf("%lld%lld%lld", (&X1[i]), (&Y1[i]), (&Z1[i]));
      scanf("%lld%lld%lld", (&X2[i]), (&Y2[i]), (&Z2[i]));
      i += 1;
    }
  }
  xsLen = cpp_assign(ysLen, "=", cpp_assign(zsLen, "=", 0));
  {
    i = 0;
    while ((i < N))
    {
      xs[cpp_update(xsLen, "++")] = X1[i];
      ys[cpp_update(ysLen, "++")] = Y1[i];
      zs[cpp_update(zsLen, "++")] = Z1[i];
      xs[cpp_update(xsLen, "++")] = X2[i];
      ys[cpp_update(ysLen, "++")] = Y2[i];
      zs[cpp_update(zsLen, "++")] = Z2[i];
      i += 1;
    }
  }
  sort(xs, (xs + xsLen));
  sort(ys, (ys + ysLen));
  sort(zs, (zs + zsLen));
  var ans = 0;
  {
    a = 0;
    while ((a < (xsLen - 1)))
    {
      {
        b = 0;
        while ((b < (ysLen - 1)))
        {
          {
            c = 0;
            while ((c < (zsLen - 1)))
            {
              var cnt = 0;
              {
                i = 0;
                while ((i < N))
                {
                  if (((((((X1[i] <= xs[a]) && (xs[(a + 1)] <= X2[i])) && (Y1[i] <= ys[b])) && (ys[(b + 1)] <= Y2[i])) && (Z1[i] <= zs[c])) && (zs[(c + 1)] <= Z2[i])))
                  {
                    cnt += 1;
                  }
                  i += 1;
                }
              }
              if ((cnt >= K))
              {
                ans += ((((xs[(a + 1)] - xs[a])) * ((ys[(b + 1)] - ys[b]))) * ((zs[(c + 1)] - zs[c])));
              }
              c += 1;
            }
          }
          b += 1;
        }
      }
      a += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
