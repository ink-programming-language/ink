// Translated from solution.cpp.

var EPS: dynamic = 1E-9;

var INF: dynamic = 1000000000;

var INF64: dynamic = cpp_cast(1E18);

var PI: dynamic = 3.1415926535897932384626433832795;

class robot
{
  var c: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
}

var d: dynamic = cpp_uninitialized();

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  if ((a.l != b.l))
  {
    return (a.l < b.l);
  }
  if ((!a.l))
  {
    return false;
  }
  return (a.f > b.f);
}

var sz: dynamic = cpp_uninitialized();

var bad: dynamic = cpp_uninitialized();

var ans1: dynamic = cpp_uninitialized();

var ans2: dynamic = cpp_uninitialized();

var t: dynamic = cpp_array(110000);

func take(t1: dynamic, f1: dynamic, t2: dynamic, f2: dynamic) -> dynamic
{
  t1 += t2;
  t2 -= min(t2, bad);
  var n: dynamic = (sz - t2);
  var pos: dynamic = int_cpp((upper_bound(t, (t + n), f2) - t));
  t1 += pos;
  if (pos)
  {
    f1 += cpp_cast(t[(pos - 1)]);
  }
  if (((t1 > ans1) || ((t1 == ans1) && (f1 < ans2))))
  {
    ans1 = t1;
    ans2 = f1;
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, d, s);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((n))))
    {
      var x: dynamic = cpp_uninitialized();
      scanf("%d%d%d", (&x.c), (&x.f), (&x.l));
      x.l = (x.l >= d);
      if (x.c)
      {
        a.push_back(x);
      } else
      {
        b.push_back(x);
      }
      i += 1;
    }
  }
  sort(a.begin(), a.end());
  sort(b.begin(), b.end());
  sz = 0;
  var sum: dynamic = 0;
  {
    var i: dynamic = (cpp_cast((b.size())) - 1);
    while ((i >= 0))
    {
      if (b[i].l)
      {
        sum += b[i].f;
        t[cpp_update(sz, "++")] = sum;
      } else
      {
        bad += 1;
      }
      i -= 1;
    }
  }
  take(0, 0, 0, s);
  var free: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((a.size()))))
    {
      free += a[i].c;
      i += 1;
    }
  }
  free -= cpp_cast(a.size());
  sum = 0;
  {
    var i: dynamic = (cpp_cast((a.size())) - 1);
    while ((i >= 0))
    {
      if ((a[i].l && (s >= a[i].f)))
      {
        sum += a[i].f;
        s -= a[i].f;
        free += 1;
        take(cpp_cast(a.size()), cpp_cast(sum), cpp_cast(min(free, cpp_cast(b.size()))), s);
      }
      i -= 1;
    }
  }
  write(ans1, cpp_char(" "), ans2, "\n");
  return 0;
}
