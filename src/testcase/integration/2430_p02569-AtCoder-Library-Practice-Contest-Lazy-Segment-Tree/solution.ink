// Translated from solution.cpp.

class LazySegmentTree
{
  var n: dynamic = cpp_uninitialized();
  var hi: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var id0: dynamic = cpp_uninitialized();
  var id1: dynamic = cpp_uninitialized();
  var dat: dynamic = cpp_uninitialized();
  var laz: dynamic = cpp_uninitialized();
  func LazySegmentTree(n: dynamic, f: dynamic, g: dynamic, h: dynamic, id0: dynamic, id1: dynamic) -> dynamic
  {
      self->f = cpp_construct(f);
      self->g = cpp_construct(g);
      self->h = cpp_construct(h);
      self->id0 = cpp_construct(id0);
      self->id1 = cpp_construct(id1);
      init(n);
    }
  func init(n: dynamic) -> dynamic
  {
      n = 1;
      hi = 0;
      while ((n < n))
      {
        n <<= 1;
        hi += 1;
      }
      dat.assign((n << 1), id0);
      laz.assign((n << 1), id1);
    }
  func build(v: dynamic) -> dynamic
  {
      {
        var i: dynamic = 0;
        while ((i < v.size()))
        {
          dat[(i + n)] = v[i];
          i += 1;
        }
      }
      {
        var i: dynamic = (n - 1);
        while (i)
        {
          dat[i] = f(dat[((i << 1) | 0)], dat[((i << 1) | 1)]);
          i -= 1;
        }
      }
    }
  func reflect(k: dynamic) -> dynamic
  {
      return  ((laz[k] == id1)) ? dat[k] : g(dat[k], laz[k]);
    }
  func propagate(k: dynamic) -> dynamic
  {
      if ((laz[k] == id1))
      {
        return;
      }
      laz[((k << 1) | 0)] = h(laz[((k << 1) | 0)], laz[k]);
      laz[((k << 1) | 1)] = h(laz[((k << 1) | 1)], laz[k]);
      dat[k] = reflect(k);
      laz[k] = id1;
    }
  func thrust(k: dynamic) -> dynamic
  {
      {
        var i: dynamic = hi;
        while (i)
        {
          propagate((k >> i));
          i -= 1;
        }
      }
    }
  func recalc(k: dynamic) -> dynamic
  {
      while (cpp_assign(k, ">>=", 1))
      {
        dat[k] = f(reflect(((k << 1) | 0)), reflect(((k << 1) | 1)));
      }
    }
  func update(a: dynamic, b: dynamic, x: dynamic) -> dynamic
  {
      if ((a >= b))
      {
        return;
      }
      thrust(cpp_assign(a, "+=", n));
      thrust(cpp_assign(b, "+=", (n - 1)));
      {
        var l: dynamic = a;
        var r: dynamic = (b + 1);
        while ((l < r))
        {
          if ((l & 1))
          {
            laz[l] = h(laz[l], x);
            l += 1;
          }
          if ((r & 1))
          {
            r -= 1;
            laz[r] = h(laz[r], x);
          }
          l >>= 1;
          r >>= 1;
        }
      }
      recalc(a);
      recalc(b);
    }
  func set_val(k: dynamic, x: dynamic) -> dynamic
  {
      thrust(cpp_assign(k, "+=", n));
      dat[k] = x;
      laz[k] = id1;
      recalc(k);
    }
  func query(a: dynamic, b: dynamic) -> dynamic
  {
      if ((a >= b))
      {
        return id0;
      }
      thrust(cpp_assign(a, "+=", n));
      thrust(cpp_assign(b, "+=", (n - 1)));
      var vl: dynamic = id0;
      var vr: dynamic = id0;
      {
        var l: dynamic = a;
        var r: dynamic = (b + 1);
        while ((l < r))
        {
          if ((l & 1))
          {
            vl = f(vl, reflect(cpp_update(l, "++")));
          }
          if ((r & 1))
          {
            vr = f(reflect(cpp_update(r, "--")), vr);
          }
          l >>= 1;
          r >>= 1;
        }
      }
      return f(vl, vr);
    }
  func find_first(a: dynamic, check: dynamic, M: dynamic, k: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      if (((l + 1) == r))
      {
        M = f(M, reflect(k));
        return  (check(M)) ? (k - n) : -1;
      }
      propagate(k);
      var m: dynamic = (((l + r)) >> 1);
      if ((m <= a))
      {
        return find_first(a, check, M, ((k << 1) | 1), m, r);
      }
      if (((a <= l) && (!check(f(M, dat[k])))))
      {
        M = f(M, dat[k]);
        return -1;
      }
      var vl: dynamic = find_first(a, check, M, ((k << 1) | 0), l, m);
      if ((~vl))
      {
        return vl;
      }
      return find_first(a, check, M, ((k << 1) | 1), m, r);
    }
  func find_first(a: dynamic, check: dynamic) -> dynamic
  {
      var M: dynamic = id0;
      return find_first(a, check, M, 1, 0, n);
    }
  func find_last(b: dynamic, check: dynamic, M: dynamic, k: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      if (((l + 1) == r))
      {
        M = f(reflect(k), M);
        return  (check(M)) ? (k - n) : -1;
      }
      propagate(k);
      var m: dynamic = (((l + r)) >> 1);
      if ((b <= m))
      {
        return find_last(b, check, M, ((k << 1) | 0), l, m);
      }
      if (((r <= b) && (!check(f(dat[k], M)))))
      {
        M = f(dat[k], M);
        return -1;
      }
      var vr: dynamic = find_last(b, check, M, ((k << 1) | 1), m, r);
      if ((~vr))
      {
        return vr;
      }
      return find_last(b, check, M, ((k << 1) | 0), l, m);
    }
  func find_last(b: dynamic, check: dynamic) -> dynamic
  {
      var M: dynamic = id0;
      return find_last(b, check, M, 1, 0, n);
    }
  func operator_index(i: dynamic) -> dynamic
  {
      return query(i, (i + 1));
    }
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic = cpp_uninitialized();
  var Q: dynamic = cpp_uninitialized();
  read(N, Q);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(A[i]);
      i += 1;
    }
  }
  cpp_statement("struct node{ int a,b; long long c,d; node(int a,int b,long long c,long long d):a(a),b(b),c(c),d(d){} }");
  var f: dynamic = __cpp_lambda_1;
  var g: dynamic = __cpp_lambda_2;
  var h: dynamic = __cpp_lambda_3;
  var seg: dynamic = cpp_construct(N, f, g, h, node(0, 0, 0, 0), 0);
  var v: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if (A[i])
      {
        v.emplace_back(0, 1, 0, 0);
      } else
      {
        v.emplace_back(1, 0, 0, 0);
      }
      i += 1;
    }
  }
  seg.build(v);
  {
    while (cpp_update(Q, "--"))
    {
      var T: dynamic = cpp_uninitialized();
      var L: dynamic = cpp_uninitialized();
      var R: dynamic = cpp_uninitialized();
      read(T, L, R);
      if ((T == 1))
      {
        seg.update(cpp_update(L, "--"), R, 1);
      } else
      {
        write(seg.query(cpp_update(L, "--"), R).d, cpp_char("\n"));
      }
    }
  }
}

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  return node((a.a + b.a), (a.b + b.b), ((a.c + b.c) + (cpp_cast(a.a) * b.b)), ((a.d + b.d) + (cpp_cast(a.b) * b.a)));
}

func __cpp_lambda_2(a: dynamic, x: dynamic) -> dynamic
{
  if ((!x))
  {
    return a;
  }
  return node(a.b, a.a, a.d, a.c);
}

func __cpp_lambda_3(a: dynamic, b: dynamic) -> dynamic
{
  return (a ^ b);
}
