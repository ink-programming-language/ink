// Translated from solution.cpp.

var maxn: dynamic = (1e6 + 7);

var dat: dynamic = cpp_array(maxn);

var minv: dynamic = cpp_array((maxn << 2));

var maxv: dynamic = cpp_array((maxn << 2));

var addv: dynamic = cpp_array((maxn << 2));

var s: dynamic = cpp_array(maxn);

func pushdown(o: dynamic) -> dynamic
{
  if ((!addv[o]))
  {
    return;
  }
  addv[((o << 1))] += addv[o];
  addv[(((o << 1) | 1))] += addv[o];
  minv[((o << 1))] += addv[o];
  minv[(((o << 1) | 1))] += addv[o];
  maxv[((o << 1))] += addv[o];
  maxv[(((o << 1) | 1))] += addv[o];
  addv[o] = 0;
}

func pushup(o: dynamic) -> dynamic
{
  minv[o] = min(minv[((o << 1))], minv[(((o << 1) | 1))]);
  maxv[o] = max(maxv[((o << 1))], maxv[(((o << 1) | 1))]);
}

func build(o: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  addv[o] = 0;
  if ((l == r))
  {
    minv[o] = dat[l];
    maxv[o] = dat[l];
    return;
  }
  var mid: dynamic = (((l + r)) >> 1);
  build(((o << 1)), l, mid);
  build((((o << 1) | 1)), (mid + 1), r);
  pushup(o);
}

func change(o: dynamic, l: dynamic, r: dynamic, ql: dynamic, qr: dynamic, v: dynamic) -> dynamic
{
  if (((ql <= l) && (qr >= r)))
  {
    minv[o] += v;
    maxv[o] += v;
    addv[o] += v;
    return;
  }
  var mid: dynamic = (((l + r)) >> 1);
  pushdown(o);
  if ((ql <= mid))
  {
    change(((o << 1)), l, mid, ql, qr, v);
  }
  if ((qr > mid))
  {
    change((((o << 1) | 1)), (mid + 1), r, ql, qr, v);
  }
  pushup(o);
}

func query_min(o: dynamic, l: dynamic, r: dynamic, ql: dynamic, qr: dynamic) -> dynamic
{
  if (((ql <= l) && (qr >= r)))
  {
    return minv[o];
  }
  var mid: dynamic = (((l + r)) >> 1);
  var ans: dynamic = 0x3f3f3f3f;
  pushdown(o);
  if ((ql <= mid))
  {
    ans = min(ans, query_min(((o << 1)), l, mid, ql, qr));
  }
  if ((qr > mid))
  {
    ans = min(ans, query_min((((o << 1) | 1)), (mid + 1), r, ql, qr));
  }
  return ans;
}

func query_max(o: dynamic, l: dynamic, r: dynamic, ql: dynamic, qr: dynamic) -> dynamic
{
  if (((ql <= l) && (qr >= r)))
  {
    return maxv[o];
  }
  var mid: dynamic = (((l + r)) >> 1);
  var ans: dynamic = -1;
  pushdown(o);
  if ((ql <= mid))
  {
    ans = max(ans, query_max(((o << 1)), l, mid, ql, qr));
  }
  if ((qr > mid))
  {
    ans = max(ans, query_max((((o << 1) | 1)), (mid + 1), r, ql, qr));
  }
  return ans;
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  build(1, 1, n);
  var a: dynamic = cpp_uninitialized();
  read(a);
  var pos: dynamic = 0;
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < a.size()))
    {
      var q: dynamic = 0;
      if ((a[i] == cpp_char("(")))
      {
        q += 1;
      } else if ((a[i] == cpp_char(")")))
      {
        q -= 1;
      }
      if ((s[pos] == cpp_char("(")))
      {
        q -= 1;
      } else if ((s[pos] == cpp_char(")")))
      {
        q += 1;
      }
      if ((a[i] == cpp_char("R")))
      {
        pos += 1;
      } else if ((a[i] == cpp_char("L")))
      {
        pos = max((pos - 1), 0);
      } else
      {
        s[pos] = a[i];
        if (q)
        {
          change(1, 1, n, (pos + 1), n, q);
        }
        sum += q;
      }
      if (((sum != 0) || (query_min(1, 1, n, 1, n) != 0)))
      {
        write("-1 ");
      } else
      {
        write(query_max(1, 1, n, 1, n), " ");
      }
      i += 1;
    }
  }
}
