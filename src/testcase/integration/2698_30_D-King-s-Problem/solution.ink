// Translated from solution.cpp.

var maxn = (100000 + 10);

var a = cpp_array(maxn);

var x: dynamic;

var y: dynamic;

var mx: dynamic;

var ans: dynamic;

func dis(i: dynamic)
{
  return sqrt(((((x - a[i])) * ((x - a[i]))) + (y * y)));
}

func cas1(l: dynamic, r: dynamic)
{
  return ((a[r] - a[l]) + min(dis(l), dis(r)));
}

func cas2(l: dynamic, r: dynamic)
{
  return ((a[r] - a[l]) + min((dis(l) + fabs((mx - a[r]))), (dis(r) + fabs((mx - a[l])))));
}

var n: dynamic;

var k: dynamic;

func main()
{
  scanf("%d%d", (&n), (&k));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lf", (&a[i]));
      i += 1;
    }
  }
  scanf("%lf%lf", (&x), (&y));
  mx = a[k];
  sort((a + 1), ((a + n) + 1));
  if ((k == (n + 1)))
  {
    ans = cas1(1, n);
  } else
  {
    ans = cas2(1, n);
    {
      var i = 1;
      while ((i < n))
      {
        ans = min(ans, min((cas1(1, i) + cas2((i + 1), n)), (cas2(1, i) + cas1((i + 1), n))));
        i += 1;
      }
    }
  }
  printf("%.10lf\n", ans);
  return 0;
}
