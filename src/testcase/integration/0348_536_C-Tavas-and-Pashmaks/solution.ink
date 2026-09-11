// Translated from solution.cpp.

var maxn: dynamic = (2e5 + 100);

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func operator_less(p: dynamic) -> dynamic
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

var pt: dynamic = cpp_array(maxn);

var stk: dynamic = cpp_array(maxn);

var stnum: dynamic = cpp_uninitialized();

var has: dynamic = cpp_uninitialized();

func check(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  return ((((c.x * b.y) * ((b.x - a.x))) * ((a.y - c.y))) < (((b.x * c.y) * ((a.x - c.x))) * ((b.y - a.y))));
}

func convex(n: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
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

var a: dynamic = cpp_array(maxn);

var b: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
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
