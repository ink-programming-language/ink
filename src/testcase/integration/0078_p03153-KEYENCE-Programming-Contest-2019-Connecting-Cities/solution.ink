// Translated from solution.cpp.

var ll = dynamic;

class node
{
  var u: dynamic;
  var v: dynamic;
  var w: dynamic;
  func operator_less(o: dynamic)
  {
      return (w < o.w);
    }
}

var e = cpp_array(6000005);

var n: dynamic;

var cot = 0;

var a = cpp_array(200005);

var fail = cpp_array(200005);

var d: dynamic;

func add(l: dynamic, r: dynamic)
{
  if ((l >= r))
  {
    return;
  }
  var m = ((l + r) >> 1);
  var mi = 1e18;
  var pos: dynamic;
  {
    var i = l;
    while ((i <= m))
    {
      var f = (a[i] - (d * i));
      if ((f < mi))
      {
        mi = f;
        pos = i;
      }
      i += 1;
    }
  }
  {
    var i = (m + 1);
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
    var i = (m + 1);
    while ((i <= r))
    {
      var f = (a[i] + (d * i));
      if ((f < mi))
      {
        mi = f;
        pos = i;
      }
      i += 1;
    }
  }
  {
    var i = l;
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

func get_fa(x: dynamic)
{
  return if ((fail[x] == x)) x else cpp_assign(fail[x], "=", get_fa(fail[x]));
}

func kruskal()
{
  sort(e, (e + cot));
  {
    var i = 1;
    while ((i <= 200000))
    {
      fail[i] = i;
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < cot))
    {
      var fu = get_fa(e[i].u);
      var fv = get_fa(e[i].v);
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

func main()
{
  scanf("%d%lld", (&n), (&d));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  add(1, n);
  printf("%lld\n", kruskal());
}
