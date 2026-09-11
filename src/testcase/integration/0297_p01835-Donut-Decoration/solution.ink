// Translated from solution.cpp.

class SegmentTree
{
  var seg: dynamic = cpp_uninitialized();
  var sz: dynamic = cpp_uninitialized();
  func SegmentTree(n: dynamic) -> dynamic
  {
      sz = 1;
      while ((sz < n))
      {
        sz <<= 1;
      }
      seg.assign(((2 * sz) - 1), make_pair(0, 0));
    }
  func push(k: dynamic) -> dynamic
  {
      if (((k >= (sz - 1)) || (seg[k] == make_pair(0, 0))))
      {
        return;
      }
      if ((seg[((2 * k) + 1)] == make_pair(0, 0)))
      {
        seg[((2 * k) + 1)] = seg[k];
      } else if ((seg[k].first == seg[((2 * k) + 1)].second))
      {
        seg[((2 * k) + 1)].second = seg[k].second;
      } else
      {
        seg[((2 * k) + 1)] = [-1, -1];
      }
      if ((seg[((2 * k) + 2)] == make_pair(0, 0)))
      {
        seg[((2 * k) + 2)] = seg[k];
      } else if ((seg[k].first == seg[((2 * k) + 2)].second))
      {
        seg[((2 * k) + 2)].second = seg[k].second;
      } else
      {
        seg[((2 * k) + 2)] = [-1, -1];
      }
      seg[k] = [0, 0];
    }
  func update(a: dynamic, b: dynamic, x: dynamic, k: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      push(k);
      if (((a >= r) || (b <= l)))
      {
      } else if (((a <= l) && (r <= b)))
      {
        if ((seg[k] == make_pair(-1, -1)))
        {
          return;
        }
        if ((k >= (sz - 1)))
        {
          if ((seg[k].second == x))
          {
            seg[k].second += 1;
          } else
          {
            seg[k] = [-1, -1];
          }
        } else
        {
          seg[k] = [x, (x + 1)];
        }
        push(k);
      } else
      {
        update(a, b, x, ((2 * k) + 1), l, (((l + r)) >> 1));
        update(a, b, x, ((2 * k) + 2), (((l + r)) >> 1), r);
      }
    }
  func update(a: dynamic, b: dynamic, x: dynamic) -> dynamic
  {
      update(a, b, x, 0, 0, sz);
    }
  func query(a: dynamic, b: dynamic, x: dynamic, k: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      push(k);
      if (((a >= r) || (b <= l)))
      {
        return (0);
      }
      if (((a <= l) && (r <= b)))
      {
        return ((seg[k] == make_pair(0, x)));
      }
      return ((query(a, b, x, ((2 * k) + 1), l, (((l + r)) >> 1)) + query(a, b, x, ((2 * k) + 2), (((l + r)) >> 1), r)));
    }
  func query(a: dynamic, b: dynamic, x: dynamic) -> dynamic
  {
      return (query(a, b, x, 0, 0, sz));
    }
}

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  var T: dynamic = cpp_uninitialized();
  scanf("%d %d", (&N), (&K));
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    scanf("%d %d %d", (&l), (&r), (&x));
    tree.update(cpp_update(l, "--"), r, cpp_update(x, "--"));
  }
  var ret: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      ret += tree.query(i, (i + 1), K);
      i += 1;
    }
  }
  printf("%d\n", ret);
}
