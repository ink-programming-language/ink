// Translated from solution.cpp.

var MAXN = (500000 + 5);

class Titem
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
}

var p = cpp_array(MAXN);

var n: dynamic;

var cmax = cpp_array(MAXN);

var tmp = cpp_array(MAXN);

var ans = 0;

func cmpb(p: dynamic, q: dynamic)
{
  return (p.b > q.b);
}

func modify(i: dynamic, v: dynamic)
{
  {
    while ((i <= n))
    {
      cmax[i] = max(cmax[i], v);
      i += (i & (-i));
    }
  }
}

func getmax(i: dynamic)
{
  var res = -1;
  {
    while (i)
    {
      res = max(res, cmax[i]);
      i -= (i & (-i));
    }
  }
  return res;
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&p[i].a));
      tmp[i] = p[i].a;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&p[i].b));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&p[i].c));
      i += 1;
    }
  }
  sort((tmp + 1), ((tmp + n) + 1));
  var last = (unique((tmp + 1), ((tmp + n) + 1)) - tmp);
  {
    var i = 1;
    while ((i <= n))
    {
      p[i].a = ((n - ((lower_bound((tmp + 1), (tmp + last), p[i].a) - tmp))) + 1);
      i += 1;
    }
  }
  sort((p + 1), ((p + n) + 1), cmpb);
  memset(cmax, -1, cpp_sizeof((cmax)));
  {
    var i = 1;
    var j = 1;
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
