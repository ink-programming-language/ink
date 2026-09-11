// Translated from solution.cpp.

var maxn: dynamic = (3e4 + 50);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var mod: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

var len: dynamic = cpp_uninitialized();

var F: dynamic = cpp_array((maxn << 1));

var sz: dynamic = cpp_uninitialized();

var bnum: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_array(maxn);

var belong: dynamic = cpp_array(maxn);

var ans: dynamic = cpp_array(maxn);

var v: dynamic = cpp_uninitialized();

class Tree
{
  var le: dynamic = cpp_uninitialized();
  var ri: dynamic = cpp_uninitialized();
  var shift: dynamic = cpp_uninitialized();
  var S1: dynamic = cpp_uninitialized();
  var S2: dynamic = cpp_uninitialized();
}

var tree: dynamic = cpp_array((maxn << 2));

func move(S1: dynamic, S2: dynamic, k: dynamic) -> dynamic
{
  var newS1: dynamic = ((((S1 * F[((len + k) - 1)]) + (S2 * F[(len + k)]))) % mod);
  var newS2: dynamic = ((((S1 * F[(len + k)]) + (S2 * F[((len + k) + 1)]))) % mod);
  S1 = newS1;
  S2 = newS2;
}

func pushup(id: dynamic) -> dynamic
{
  tree[id].S1 = (((tree[(id << 1)].S1 + tree[((id << 1) | 1)].S1)) % mod);
  tree[id].S2 = (((tree[(id << 1)].S2 + tree[((id << 1) | 1)].S2)) % mod);
}

func pushdown(id: dynamic) -> dynamic
{
  if (tree[id].shift)
  {
    tree[(id << 1)].shift += tree[id].shift;
    move(tree[(id << 1)].S1, tree[(id << 1)].S2, tree[id].shift);
    tree[((id << 1) | 1)].shift += tree[id].shift;
    move(tree[((id << 1) | 1)].S1, tree[((id << 1) | 1)].S2, tree[id].shift);
    tree[id].shift = 0;
  }
}

func build(id: dynamic, le: dynamic, ri: dynamic) -> dynamic
{
  tree[id].le = le;
  tree[id].ri = ri;
  tree[id].shift = cpp_assign(tree[id].S1, "=", cpp_assign(tree[id].S2, "=", 0));
  if ((le == ri))
  {
    return;
  }
  var mid: dynamic = (((le + ri)) >> 1);
  build((id << 1), le, mid);
  build(((id << 1) | 1), (mid + 1), ri);
}

func Insert(id: dynamic, pos: dynamic, val: dynamic) -> dynamic
{
  if ((tree[id].le == tree[id].ri))
  {
    tree[id].S1 = (((1 * val) * F[(len + tree[id].shift)]) % mod);
    tree[id].S2 = (((1 * val) * F[((len + tree[id].shift) + 1)]) % mod);
    return;
  }
  pushdown(id);
  var mid: dynamic = (((tree[id].le + tree[id].ri)) >> 1);
  if ((pos <= mid))
  {
    tree[((id << 1) | 1)].shift += 1;
    move(tree[((id << 1) | 1)].S1, tree[((id << 1) | 1)].S2, 1);
    Insert((id << 1), pos, val);
  } else
  {
    Insert(((id << 1) | 1), pos, val);
  }
  pushup(id);
}

func Remove(id: dynamic, pos: dynamic) -> dynamic
{
  if ((tree[id].le == tree[id].ri))
  {
    tree[id].S1 = cpp_assign(tree[id].S2, "=", 0);
    return;
  }
  pushdown(id);
  var mid: dynamic = (((tree[id].le + tree[id].ri)) >> 1);
  if ((pos <= mid))
  {
    tree[((id << 1) | 1)].shift -= 1;
    move(tree[((id << 1) | 1)].S1, tree[((id << 1) | 1)].S2, -1);
    Remove((id << 1), pos);
  } else
  {
    Remove(((id << 1) | 1), pos);
  }
  pushup(id);
}

class Node
{
  var id: dynamic = cpp_uninitialized();
  var le: dynamic = cpp_uninitialized();
  var ri: dynamic = cpp_uninitialized();
}

var q: dynamic = cpp_array(maxn);

func cmp(x: dynamic, y: dynamic) -> dynamic
{
  return  (((belong[x.le] ^ belong[y.le]))) ? (belong[x.le] < belong[y.le]) : ( (((belong[x.le] & 1))) ? (x.ri < y.ri) : (x.ri > y.ri));
}

func Add(pos: dynamic) -> dynamic
{
  pos = ((lower_bound(v.begin(), v.end(), a[pos]) - v.begin()) + 1);
  if (cpp_update((!cnt[pos]), "++"))
  {
    Insert(1, pos, v[(pos - 1)]);
  }
}

func Del(pos: dynamic) -> dynamic
{
  pos = ((lower_bound(v.begin(), v.end(), a[pos]) - v.begin()) + 1);
  if ((!cpp_update(cnt[pos], "--")))
  {
    Remove(1, pos);
  }
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&mod));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      v.push_back(a[i]);
      i += 1;
    }
  }
  sort(v.begin(), v.end());
  v.erase(unique(v.begin(), v.end()), v.end());
  len = v.size();
  F[len] = 0;
  F[(len + 1)] = 1;
  {
    var i: dynamic = 2;
    while (((i + len) < ((maxn << 1))))
    {
      F[(i + len)] = (((F[((i + len) - 1)] + F[((i + len) - 2)])) % mod);
      i += 1;
    }
  }
  {
    var i: dynamic = -1;
    while (((i + len) >= 0))
    {
      F[(i + len)] = ((((F[((i + len) + 2)] - F[((i + len) + 1)]) + mod)) % mod);
      i -= 1;
    }
  }
  build(1, 1, len);
  sz = sqrt(n);
  bnum = ceil(cpp_double((n / sz)));
  {
    var i: dynamic = 1;
    while ((i <= bnum))
    {
      {
        var j: dynamic = ((((i - 1)) * sz) + 1);
        while (((j <= (i * sz)) && (j <= n)))
        {
          belong[j] = i;
          j += 1;
        }
      }
      i += 1;
    }
  }
  scanf("%d", (&m));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d%d", (&q[i].le), (&q[i].ri));
      q[i].id = i;
      i += 1;
    }
  }
  sort(q, (q + m), cmp);
  var L: dynamic = 1;
  var R: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      while ((L < q[i].le))
      {
        Del(cpp_update(L, "++"));
      }
      while ((L > q[i].le))
      {
        Add(cpp_update(L, "--"));
      }
      while ((R < q[i].ri))
      {
        Add(cpp_update(R, "++"));
      }
      while ((R > q[i].ri))
      {
        Del(cpp_update(R, "--"));
      }
      ans[q[i].id] = tree[1].S2;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      printf("%d\n", ans[i]);
      i += 1;
    }
  }
  return 0;
}
