// Translated from solution.cpp.

var MAXN: dynamic = (500000 + 5);

class Titem
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
}

var p: dynamic = cpp_array(MAXN);

var n: dynamic = cpp_uninitialized();

var cmax: dynamic = cpp_array(MAXN);

var tmp: dynamic = cpp_array(MAXN);

var ans: dynamic = 0;

func cmpb(p: dynamic, q: dynamic) -> dynamic
{
  return (p.b > q.b);
}

func modify(i: dynamic, v: dynamic) -> dynamic
{
  {
    while ((i <= n))
    {
      cmax[i] = max(cmax[i], v);
      i += (i & (-i));
    }
  }
}

func getmax(i: dynamic) -> dynamic
{
  var res: dynamic = -1;
  {
    while (i)
    {
      res = max(res, cmax[i]);
      i -= (i & (-i));
    }
  }
  return res;
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&p[i].a));
      tmp[i] = p[i].a;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&p[i].b));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&p[i].c));
      i += 1;
    }
  }
  sort((tmp + 1), ((tmp + n) + 1));
  var last: dynamic = (unique((tmp + 1), ((tmp + n) + 1)) - tmp);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      p[i].a = ((n - ((lower_bound((tmp + 1), (tmp + last), p[i].a) - tmp))) + 1);
      i += 1;
    }
  }
  sort((p + 1), ((p + n) + 1), cmpb);
  memset(cmax, -1, cpp_sizeof((cmax)));
  {
    var i: dynamic = 1;
    var j: dynamic = 1;
    while ((i <= n))
    {
      {
        while ((p[j].b > p[i].b))
        {
          modify(p[j].a, p[j].c);
          j += 1;
        }
      }
      ans += (getmax((p[i].a - 1)) > p[i].c);
      i += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
