// Translated from solution.cpp.

var EPS = 1E-9;

var INF = 1000000000;

var INF64 = cpp_cast(1E18);

var PI = 3.1415926535897932384626433832795;

class robot
{
  var c: dynamic;
  var f: dynamic;
  var l: dynamic;
}

var d: dynamic;

func operator_less(a: dynamic, b: dynamic)
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

var sz: dynamic;

var bad: dynamic;

var ans1: dynamic;

var ans2: dynamic;

var t = cpp_array(110000);

func take(t1: dynamic, f1: dynamic, t2: dynamic, f2: dynamic)
{
  t1 += t2;
  t2 -= min(t2, bad);
  var n = (sz - t2);
  var pos = int_cpp((upper_bound(t, (t + n), f2) - t));
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

func main()
{
  var n: dynamic;
  var s: dynamic;
  read(n, d, s);
  var a: dynamic;
  var b: dynamic;
  {
    var i = 0;
    while ((i < cpp_cast((n))))
    {
      var x: dynamic;
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
  var sum = 0;
  {
    var i = (cpp_cast((b.size())) - 1);
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
  var free = 0;
  {
    var i = 0;
    while ((i < cpp_cast((a.size()))))
    {
      free += a[i].c;
      i += 1;
    }
  }
  free -= cpp_cast(a.size());
  sum = 0;
  {
    var i = (cpp_cast((a.size())) - 1);
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
