// Translated from solution.cpp.

var maxn = (2e5 + 100);

class Point
{
  var x: dynamic;
  var y: dynamic;
  func operator_less(p: dynamic)
  {
      if ((x != p.x))
      {
        return (x > p.x);
      } else
      {
        return (y > p.y);
      }
    }
}

var pt = cpp_array(maxn);

var stk = cpp_array(maxn);

var stnum: dynamic;

var has: dynamic;

func check(a: dynamic, b: dynamic, c: dynamic)
{
  return ((((c.x * b.y) * ((b.x - a.x))) * ((a.y - c.y))) < (((b.x * c.y) * ((a.x - c.x))) * ((b.y - a.y))));
}

func convex(n: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  stnum = 0;
  {
    i = 0;
    while ((i < n))
    {
      if (((stnum > 0) && (pt[i].y <= pt[stk[(stnum - 1)]].y)))
      {
        i += 1;
        continue;
      }
      while (((stnum > 1) && check(pt[stk[(stnum - 1)]], pt[stk[(stnum - 2)]], pt[i])))
      {
        stnum -= 1;
      }
      stk[cpp_update(stnum, "++")] = i;
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < stnum))
    {
      has.insert(make_pair(pt[stk[i]].x, pt[stk[i]].y));
      i += 1;
    }
  }
}

var a = cpp_array(maxn);

var b = cpp_array(maxn);

func main()
{
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
  scanf("%d", (&n));
  {
    i = 0;
    while ((i < n))
    {
      scanf("%I64d %I64d", (&a[i]), (&b[i]));
      pt[i].x = a[i];
      pt[i].y = b[i];
      i += 1;
    }
  }
  sort(pt, (pt + n));
  convex(n);
  {
    i = 0;
    while ((i < n))
    {
      if ((has.find(make_pair(a[i], b[i])) != has.end()))
      {
        printf("%d ", (i + 1));
      }
      i += 1;
    }
  }
  printf("\n");
  return 0;
}
