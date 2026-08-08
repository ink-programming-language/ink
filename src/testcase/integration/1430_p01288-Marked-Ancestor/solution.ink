// Translated from solution.cpp.

var ll = dynamic;

var q: dynamic;

var n: dynamic;

var v: dynamic;

var c: dynamic;

var p = cpp_array(100005);

func main()
{
  while (((((~scanf("%d %d", (&n), (&q)))) && q) && n))
  {
    {
      var i = 2;
      while ((i <= n))
      {
        scanf("%d", (&p[i]));
        i += 1;
      }
    }
    p[1] = 1;
    var ans = 0;
    {
      var i = 0;
      while ((i < q))
      {
        scanf("\n%c %d", (&c), (&v));
        if ((c == cpp_char("M")))
        {
          p[v] = v;
        } else
        {
          while ((p[v] != v))
          {
            v = p[v];
          }
          ans += v;
        }
        i += 1;
      }
    }
    printf("%lld\n", ans);
  }
  return 0;
}
