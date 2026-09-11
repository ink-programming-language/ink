// Translated from solution.cpp.

var ll: dynamic = dynamic;

class node
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  func operator_less(o: dynamic) -> dynamic
  {
      return (w < o.w);
    }
}

var e: dynamic = cpp_array(6000005);

var n: dynamic = cpp_uninitialized();

var cot: dynamic = 0;

var a: dynamic = cpp_array(200005);

var fail: dynamic = cpp_array(200005);

var d: dynamic = cpp_uninitialized();

func add(l: dynamic, r: dynamic) -> dynamic
{
  if ((l >= r))
  {
    return;
  }
  var m: dynamic = ((l + r) >> 1);
  var mi: dynamic = 1e18;
  var pos: dynamic = cpp_uninitialized();
  {
    var i: dynamic = l;
    while ((i <= m))
    {
      var f: dynamic = (a[i] - (d * i));
      if ((f < mi))
      {
        mi = f;
        pos = i;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = (m + 1);
    while ((i <= r))
    {
      e[cot].u = pos;
      e[cot].v = i;
      e[cpp_update(cot, "++")].w = ((a[i] + a[pos]) + (d * ((i - pos))));
      i += 1;
    }
  }
  mi = 1e18;
  {
    var i: dynamic = (m + 1);
    while ((i <= r))
    {
      var f: dynamic = (a[i] + (d * i));
      if ((f < mi))
      {
        mi = f;
        pos = i;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = l;
    while ((i <= m))
    {
      e[cot].u = i;
      e[cot].v = pos;
      e[cpp_update(cot, "++")].w = ((a[i] + a[pos]) + (d * ((pos - i))));
      i += 1;
    }
  }
  add(l, m);
  add((m + 1), r);
}

func get_fa(x: dynamic) -> dynamic
{
  return  ((fail[x] == x)) ? x : cpp_assign(fail[x], "=", get_fa(fail[x]));
}

func kruskal() -> dynamic
{
  sort(e, (e + cot));
  {
    var i: dynamic = 1;
    while ((i <= 200000))
    {
      fail[i] = i;
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < cot))
    {
      var fu: dynamic = get_fa(e[i].u);
      var fv: dynamic = get_fa(e[i].v);
      if ((fu == fv))
      {
        i += 1;
        continue;
      }
      fail[fu] = fv;
      ans += e[i].w;
      i += 1;
    }
  }
  return ans;
}

func main() -> dynamic
{
  scanf("%d%lld", (&n), (&d));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  add(1, n);
  printf("%lld\n", kruskal());
}
