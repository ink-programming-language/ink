// Translated from solution.cpp.

class T
{
  var sum: dynamic = cpp_uninitialized();
  var len: dynamic = cpp_uninitialized();
  var sol: dynamic = cpp_uninitialized();
}

func unite(a: dynamic, b: dynamic) -> dynamic
{
  var sum: dynamic = (a.sum + b.sum);
  var len: dynamic = (a.len + b.len);
  var sol: dynamic = (((a.sol + b.sol) + (b.sum * a.len)) - (a.sum * b.len));
  return [sum, len, sol];
}

var N: dynamic = (cpp_cast(1e5) + 7);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var grow: dynamic = cpp_array(N);

var m: dynamic = cpp_uninitialized();

var trans: dynamic = cpp_uninitialized();

var tp: dynamic = cpp_array(N);

var x: dynamic = cpp_array(N);

var y: dynamic = cpp_array(N);

var now: dynamic = cpp_uninitialized();

var aint: dynamic = cpp_array(((4 * N) * 3));

func del(v: dynamic, tl: dynamic, tr: dynamic, pos: dynamic) -> dynamic
{
  if (((tr < pos) || (pos < tl)))
  {
    return;
  }
  if ((tl == tr))
  {
    aint[v] = [0, 0, 0];
  } else
  {
    var tm: dynamic = (((tl + tr)) / 2);
    del((2 * v), tl, tm, pos);
    del(((2 * v) + 1), (tm + 1), tr, pos);
    aint[v] = unite(aint[(2 * v)], aint[((2 * v) + 1)]);
  }
}

func make(v: dynamic, tl: dynamic, tr: dynamic, pos: dynamic, x: dynamic) -> dynamic
{
  if (((tr < pos) || (pos < tl)))
  {
    return;
  }
  if ((tl == tr))
  {
    aint[v] = [x, 1, 0];
  } else
  {
    var tm: dynamic = (((tl + tr)) / 2);
    make((2 * v), tl, tm, pos, x);
    make(((2 * v) + 1), (tm + 1), tr, pos, x);
    aint[v] = unite(aint[(2 * v)], aint[((2 * v) + 1)]);
  }
}

func get(v: dynamic, tl: dynamic, tr: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if (((tr < l) || (r < tl)))
  {
    return [0, 0, 0];
  }
  if (((l <= tl) && (tr <= r)))
  {
    return aint[v];
  } else
  {
    var tm: dynamic = (((tl + tr)) / 2);
    return unite(get((2 * v), tl, tm, l, r), get(((2 * v) + 1), (tm + 1), tr, l, r));
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      trans[a[i]] = 0;
      i += 1;
    }
  }
  read(m);
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      read(tp[i], x[i], y[i]);
      if ((tp[i] == 1))
      {
        grow[x[i]] += y[i];
        trans[(a[x[i]] + grow[x[i]])] = 0;
      } else
      {
        trans[x[i]] = 0;
        trans[y[i]] = 0;
      }
      i += 1;
    }
  }
  for (var it: dynamic in trans)
  {
    it.second = cpp_update(now, "++");
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      make(1, 1, now, trans[a[i]], a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      if ((tp[i] == 1))
      {
        del(1, 1, now, trans[a[x[i]]]);
        a[x[i]] += y[i];
        make(1, 1, now, trans[a[x[i]]], a[x[i]]);
      } else
      {
        var it: dynamic = get(1, 1, now, trans[x[i]], trans[y[i]]);
        write(it.sol, "\n");
      }
      i += 1;
    }
  }
}
