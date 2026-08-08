// Translated from solution.cpp.

var n: dynamic;

var w: dynamic;

var ans: dynamic;

class plane
{
  var a: dynamic;
  var b: dynamic;
}

var p = cpp_array(100005);

var temp = cpp_array(100005);

func cmp(x: dynamic, y: dynamic)
{
  return ((x.a < y.a) || (((x.a == y.a) && (x.b > y.b))));
}

func CDQ(l: dynamic, r: dynamic)
{
  if ((l == r))
  {
    return;
  }
  var mid = (((l + r)) >> 1);
  CDQ(l, mid);
  CDQ((mid + 1), r);
  {
    var i = l;
    while ((i <= r))
    {
      temp[i] = p[i];
      i += 1;
    }
  }
  var cnt = l;
  {
    var i = l;
    var j = (mid + 1);
    while (((i <= mid) || (j <= r)))
    {
      if (((((j > r) || (temp[i].b < temp[j].b))) && (i <= mid)))
      {
        p[cpp_update(cnt, "++")] = temp[cpp_update(i, "++")];
      } else
      {
        p[cpp_update(cnt, "++")] = temp[cpp_update(j, "++")];
        ans += cpp_cast((((mid - i) + 1)));
      }
    }
  }
}

func main()
{
  scanf("%d %d", (&n), (&w));
  {
    var i = 1;
    var x: dynamic;
    var v: dynamic;
    while ((i <= n))
    {
      scanf("%d %d", (&x), (&v));
      p[i].a = fabs((cpp_cast(x) / cpp_cast(((v + w)))));
      p[i].b = fabs((cpp_cast(x) / cpp_cast(((v - w)))));
      i += 1;
    }
  }
  sort((p + 1), ((p + 1) + n), cmp);
  CDQ(1, n);
  printf("%lld\n", ans);
  return 0;
}
