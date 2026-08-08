// Translated from solution.cpp.

var MOD = 998244353;

class tnode
{
  var sum: dynamic;
  var lson: dynamic;
  var rson: dynamic;
  func tnode(x: dynamic = 0)
  {
      sum = x;
      lson = cpp_assign(rson, "=", null);
    }
}

func pushup(cur: dynamic)
{
  cur->sum = ((if ((cur->lson == null)) 0 else cur->lson->sum) + (if ((cur->rson == null)) 0 else cur->rson->sum));
}

func modify(cur: dynamic, id: dynamic, val: dynamic, cl: dynamic = 0, cr: dynamic = 1048575)
{
  if ((cl == cr))
  {
    return cpp_new(val);
  }
  var mid = (((cl + cr)) >> 1);
  var ret = cpp_new();
  var ls = if ((cur == null)) null else cur->lson;
  var rs = if ((cur == null)) null else cur->rson;
  ret->lson = if ((id <= mid)) modify(ls, id, val, cl, mid) else ls;
  ret->rson = if ((id > mid)) modify(rs, id, val, (mid + 1), cr) else rs;
  pushup(ret);
  return ret;
}

func query(cur: dynamic, l: dynamic, r: dynamic, cl: dynamic = 0, cr: dynamic = 1048575)
{
  if ((cur == null))
  {
    return 0;
  }
  if (((l == cl) && (r == cr)))
  {
    return cur->sum;
  }
  var mid = (((cl + cr)) >> 1);
  if ((r <= mid))
  {
    return query(cur->lson, l, r, cl, mid);
  } else if ((l > mid))
  {
    return query(cur->rson, l, r, (mid + 1), cr);
  } else
  {
    return (query(cur->lson, l, mid, cl, mid) + query(cur->rson, (mid + 1), r, (mid + 1), cr));
  }
}

var n: dynamic;

var q: dynamic;

var p0 = cpp_array(1000005);

var occ = cpp_array(1000005);

var p1 = cpp_array(1000005);

var tre: dynamic;

var x: dynamic;

func f(z: dynamic)
{
  return (((((z - 1) + x)) % n) + 1);
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= cpp_cast((n))))
    {
      scanf("%d", (&p0[i]));
      occ[p0[i]] = i;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= cpp_cast((n))))
    {
      scanf("%d", (&p1[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= cpp_cast((n))))
    {
      tre[i] = modify(tre[(i - 1)], occ[p1[i]], 1);
      i += 1;
    }
  }
  scanf("%d", (&q));
  x = 0;
  {
    var i = 0;
    while ((i < cpp_cast((q))))
    {
      var a: dynamic;
      var b: dynamic;
      var c: dynamic;
      var d: dynamic;
      scanf("%d%d%d%d", (&a), (&b), (&c), (&d));
      a = f(a);
      b = f(b);
      c = f(c);
      d = f(d);
      if ((a > b))
      {
        swap(a, b);
      }
      if ((c > d))
      {
        swap(c, d);
      }
      x = ((query(tre[d], a, b) - query(tre[(c - 1)], a, b)) + 1);
      printf("%d\n", (x - 1));
      i += 1;
    }
  }
  return 0;
}
