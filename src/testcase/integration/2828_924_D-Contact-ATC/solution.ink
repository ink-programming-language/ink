// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

class plane
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
}

var p: dynamic = cpp_array(100005);

var temp: dynamic = cpp_array(100005);

func cmp(x: dynamic, y: dynamic) -> dynamic
{
  return ((x.a < y.a) || (((x.a == y.a) && (x.b > y.b))));
}

func CDQ(l: dynamic, r: dynamic) -> dynamic
{
  if ((l == r))
  {
    return;
  }
  var mid: dynamic = (((l + r)) >> 1);
  CDQ(l, mid);
  CDQ((mid + 1), r);
  {
    var i: dynamic = l;
    while ((i <= r))
    {
      temp[i] = p[i];
      i += 1;
    }
  }
  var cnt: dynamic = l;
  {
    var i: dynamic = l;
    var j: dynamic = (mid + 1);
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

func main() -> dynamic
{
  scanf("%d %d", (&n), (&w));
  {
    var i: dynamic = 1;
    var x: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
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
