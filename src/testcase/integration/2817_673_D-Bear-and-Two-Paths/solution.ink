// Translated from solution.cpp.

var v = cpp_array(1001);

var u = cpp_array(1001);

var n: dynamic;

var a: dynamic;

var b: dynamic;

var c: dynamic;

var d: dynamic;

var k: dynamic;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  scanf("%d%d%d%d%d%d", (&n), (&k), (&a), (&b), (&c), (&d));
  if ((n == 5))
  {
    if ((k <= 5))
    {
      printf("-1");
      return 0;
    }
    var e = (15 - ((((a + b) + c) + d)));
    printf("%d %d %d %d %d\n%d %d %d %d %d", a, d, e, c, b, c, b, e, a, d);
    return 0;
  }
  if (((n == 4) || (k <= n)))
  {
    printf("-1");
    return 0;
  }
  var e = 1;
  while (((((e == a) || (e == b)) || (e == c)) || (e == d)))
  {
    e += 1;
  }
  v[1] = a;
  v[2] = c;
  v[3] = e;
  v[4] = d;
  var cnt = 4;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((((((i - a) && (i - b)) && (i - c)) && (i - d)) && (i - e)))
      {
        v[cpp_update(cnt, "++")] = i;
      }
      i += 1;
    }
  }
  v[n] = b;
  {
    var i = 1;
    while ((i <= n))
    {
      printf("%d ", v[i]);
      i += 1;
    }
  }
  u[1] = c;
  u[2] = a;
  u[3] = e;
  u[4] = b;
  cnt = 4;
  {
    var i = (n - 1);
    while ((i >= 4))
    {
      u[cpp_update(cnt, "++")] = v[i];
      i -= 1;
    }
  }
  printf("\n");
  {
    var i = 1;
    while ((i <= n))
    {
      printf("%d ", u[i]);
      i += 1;
    }
  }
  return 0;
}
