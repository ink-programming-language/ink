// Translated from solution.cpp.

var ll: dynamic = dynamic;

var q: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(100005);

func main() -> dynamic
{
  while (((((~scanf("%d %d", (&n), (&q)))) && q) && n))
  {
    {
      var i: dynamic = 2;
      while ((i <= n))
      {
        scanf("%d", (&p[i]));
        i += 1;
      }
    }
    p[1] = 1;
    var ans: dynamic = 0;
    {
      var i: dynamic = 0;
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
