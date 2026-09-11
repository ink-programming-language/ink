// Translated from solution.cpp.

var MOD: dynamic = 998244353;

class tnode
{
  var sum: dynamic = cpp_uninitialized();
  var lson: dynamic = cpp_uninitialized();
  var rson: dynamic = cpp_uninitialized();
  func tnode(x: dynamic = 0) -> dynamic
  {
      sum = x;
      lson = cpp_assign(rson, "=", null);
    }
}

func pushup(cur: dynamic) -> dynamic
{
  cur->sum = (( ((cur->lson == null)) ? 0 : cur->lson->sum) + ( ((cur->rson == null)) ? 0 : cur->rson->sum));
}

func modify(cur: dynamic, id: dynamic, val: dynamic, cl: dynamic = 0, cr: dynamic = 1048575) -> dynamic
{
  if ((cl == cr))
  {
    return cpp_new(val);
  }
  var mid: dynamic = (((cl + cr)) >> 1);
  var ret: dynamic = cpp_new();
  var ls: dynamic =  ((cur == null)) ? null : cur->lson;
  var rs: dynamic =  ((cur == null)) ? null : cur->rson;
  ret->lson =  ((id <= mid)) ? modify(ls, id, val, cl, mid) : ls;
  ret->rson =  ((id > mid)) ? modify(rs, id, val, (mid + 1), cr) : rs;
  pushup(ret);
  return ret;
}

func query(cur: dynamic, l: dynamic, r: dynamic, cl: dynamic = 0, cr: dynamic = 1048575) -> dynamic
{
  if ((cur == null))
  {
    return 0;
  }
  if (((l == cl) && (r == cr)))
  {
    return cur->sum;
  }
  var mid: dynamic = (((cl + cr)) >> 1);
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

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var p0: dynamic = cpp_array(1000005);

var occ: dynamic = cpp_array(1000005);

var p1: dynamic = cpp_array(1000005);

var tre: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

func f(z: dynamic) -> dynamic
{
  return (((((z - 1) + x)) % n) + 1);
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= cpp_cast((n))))
    {
      scanf("%d", (&p0[i]));
      occ[p0[i]] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= cpp_cast((n))))
    {
      scanf("%d", (&p1[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= cpp_cast((n))))
    {
      tre[i] = modify(tre[(i - 1)], occ[p1[i]], 1);
      i += 1;
    }
  }
  scanf("%d", (&q));
  x = 0;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((q))))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      var c: dynamic = cpp_uninitialized();
      var d: dynamic = cpp_uninitialized();
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
