// Translated from solution.cpp.

var maxn: dynamic = (1e5 + 10);

var b: dynamic = cpp_array((maxn << 3));

var id: dynamic = cpp_array((maxn << 3));

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

class seg
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var val: dynamic = cpp_uninitialized();

func pos(x: dynamic) -> dynamic
{
  return ((lower_bound(val.begin(), val.end(), x) - val.begin()) + 1);
}

func build(o: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if ((r == l))
  {
    b[o] = -1e9;
    return;
  }
  build(((o * 2)), l, ((((l + r)) >> 1)));
  build((((o * 2) + 1)), (((((l + r)) >> 1)) + 1), r);
  b[o] = max(b[((o * 2))], b[(((o * 2) + 1))]);
}

func A(o: dynamic, l: dynamic, r: dynamic, qd: dynamic, d1: dynamic, d2: dynamic) -> dynamic
{
  if ((r == l))
  {
    b[o] = d1;
    id[o] = d2;
    return;
  }
  if ((qd <= ((((l + r)) >> 1))))
  {
    A(((o * 2)), l, ((((l + r)) >> 1)), qd, d1, d2);
  } else
  {
    A((((o * 2) + 1)), (((((l + r)) >> 1)) + 1), r, qd, d1, d2);
  }
  b[o] = max(b[((o * 2))], b[(((o * 2) + 1))]);
}

func Q(o: dynamic, l: dynamic, r: dynamic, qd: dynamic, d: dynamic) -> dynamic
{
  if ((b[o] < d))
  {
    return -1;
  }
  var ans: dynamic = cpp_uninitialized();
  if ((r == l))
  {
    return id[o];
  }
  if ((qd <= ((((l + r)) >> 1))))
  {
    ans = Q(((o * 2)), l, ((((l + r)) >> 1)), qd, d);
    if ((ans != -1))
    {
      return ans;
    }
  }
  return Q((((o * 2) + 1)), (((((l + r)) >> 1)) + 1), r, qd, d);
}

var ans: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  var v: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= (n + m)))
    {
      var l: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      scanf("%d%d%d", (&l), (&r), (&t));
      v.push_back([l, r, t, i]);
      val.push_back(t);
      i += 1;
    }
  }
  sort(val.begin(), val.end());
  val.resize((unique(val.begin(), val.end()) - val.begin()));
  sort(v.begin(), v.end(), __cpp_lambda_1);
  build(1, 1, val.size());
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      if ((v[i].id <= n))
      {
        A(1, 1, val.size(), pos(v[i].t), v[i].r, v[i].id);
      } else
      {
        ans[(v[i].id - n)] = Q(1, 1, val.size(), pos(v[i].t), v[i].r);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      printf("%d%c", ans[i], " \n"[(i == n)]);
      i += 1;
    }
  }
}

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  if ((a.l == b.l))
  {
    return (a.id < b.id);
  }
  return (a.l < b.l);
}
