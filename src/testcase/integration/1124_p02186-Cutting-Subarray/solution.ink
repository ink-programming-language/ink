// Translated from solution.cpp.

var F: dynamic = cpp_expression("#incl");

var S: dynamic = cpp_expression("#inclu");

var MOD: dynamic = 1000000007;

func operator_shift_right(i: dynamic, A: dynamic) -> dynamic
{
  ((i >> A.F) >> A.S);
  return i;
}

func operator_shift_right(i: dynamic, A: dynamic) -> dynamic
{
  for (var I: dynamic in A)
  {
    (i >> I);
  }
  return i;
}

func operator_shift_left(o: dynamic, A: dynamic) -> dynamic
{
  (((o << A.F) << " ") << A.S);
  return o;
}

func operator_shift_left(o: dynamic, A: dynamic) -> dynamic
{
  var i: dynamic = A.size();
  for (var I: dynamic in A)
  {
    ((o << I) << ( (cpp_update(i, "--")) ? " " : ""));
  }
  return o;
}

class SegmentTree
{
  var n: dynamic = cpp_uninitialized();
  var height: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var ti: dynamic = cpp_uninitialized();
  var ei: dynamic = cpp_uninitialized();
  var dat: dynamic = cpp_uninitialized();
  var laz: dynamic = cpp_uninitialized();
  func SegmentTree(f: dynamic, g: dynamic, h: dynamic, ti: dynamic, ei: dynamic) -> dynamic
  {
      self->f = cpp_construct(f);
      self->g = cpp_construct(g);
      self->h = cpp_construct(h);
      self->ti = cpp_construct(ti);
      self->ei = cpp_construct(ei);
    }
  func init(n: dynamic) -> dynamic
  {
      n = 1;
      height = 0;
      while ((n < n))
      {
        n <<= 1;
        height += 1;
      }
      dat.assign((2 * n), ti);
      laz.assign((2 * n), ei);
    }
  func build(v: dynamic) -> dynamic
  {
      var n: dynamic = v.size();
      init(n);
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          dat[(n + i)] = v[i];
          i += 1;
        }
      }
      {
        var i: dynamic = (n - 1);
        while (i)
        {
          dat[i] = f(dat[(((i << 1)) | 0)], dat[(((i << 1)) | 1)]);
          i -= 1;
        }
      }
    }
  func reflect(k: dynamic) -> dynamic
  {
      return  ((laz[k] == ei)) ? dat[k] : g(dat[k], laz[k]);
    }
  func eval(k: dynamic) -> dynamic
  {
      if ((laz[k] == ei))
      {
        return;
      }
      laz[(((k << 1)) | 0)] = h(laz[(((k << 1)) | 0)], laz[k]);
      laz[(((k << 1)) | 1)] = h(laz[(((k << 1)) | 1)], laz[k]);
      dat[k] = reflect(k);
      laz[k] = ei;
    }
  func thrust(k: dynamic) -> dynamic
  {
      {
        var i: dynamic = height;
        while (i)
        {
          eval((k >> i));
          i -= 1;
        }
      }
    }
  func recalc(k: dynamic) -> dynamic
  {
      while (cpp_assign(k, ">>=", 1))
      {
        dat[k] = f(reflect((((k << 1)) | 0)), reflect((((k << 1)) | 1)));
      }
    }
  func update(a: dynamic, b: dynamic, x: dynamic) -> dynamic
  {
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
  func set_val(a: dynamic, x: dynamic) -> dynamic
  {
      thrust(cpp_assign(a, "+=", n));
      dat[a] = x;
      laz[a] = ei;
      recalc(a);
    }
  func query(a: dynamic, b: dynamic) -> dynamic
  {
      thrust(cpp_assign(a, "+=", n));
      thrust(cpp_assign(b, "+=", (n - 1)));
      var vl: dynamic = ti;
      var vr: dynamic = ti;
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
  func find(st: dynamic, check: dynamic, acc: dynamic, k: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      if (((l + 1) == r))
      {
        acc = f(acc, reflect(k));
        return  (check(acc)) ? (k - n) : -1;
      }
      eval(k);
      var m: dynamic = (((l + r)) >> 1);
      if ((m <= st))
      {
        return find(st, check, acc, (((k << 1)) | 1), m, r);
      }
      if (((st <= l) && (!check(f(acc, dat[k])))))
      {
        acc = f(acc, dat[k]);
        return -1;
      }
      var vl: dynamic = find(st, check, acc, (((k << 1)) | 0), l, m);
      if ((~vl))
      {
        return vl;
      }
      return find(st, check, acc, (((k << 1)) | 1), m, r);
    }
  func find(st: dynamic, check: dynamic) -> dynamic
  {
      var acc: dynamic = ti;
      return find(st, check, acc, 1, 0, n);
    }
}

class node
{
  var mi: dynamic = cpp_uninitialized();
  var mx: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  var sum: dynamic = cpp_uninitialized();
  func node(a: dynamic = 0, b: dynamic = 0, c: dynamic = 0, d: dynamic = 0) -> dynamic
  {
      self->mi = cpp_construct(a);
      self->mx = cpp_construct(b);
      self->ans = cpp_construct(c);
      self->sum = cpp_construct(d);
    }
  func operator_equal(A: dynamic) -> dynamic
  {
      return ((((A.mi == mi) && (A.mx == mx)) && (A.ans == ans)) && (A.sum == sum));
    }
}

var uku: dynamic = (-1 * MOD);

var err: dynamic = node(MOD, uku, 0, 0);

func main() -> dynamic
{
  var f: dynamic = __cpp_lambda_1;
  var g: dynamic = __cpp_lambda_2;
  var h: dynamic = __cpp_lambda_3;
  var N: dynamic = cpp_uninitialized();
  var Q: dynamic = cpp_uninitialized();
  read(N, Q);
  N += 1;
  ch.build(vector(N, node()));
  var A: dynamic = cpp_construct((N - 1));
  read(A);
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      ch.update(i, (i + 1), node(A[(i - 1)], A[(i - 1)], 0, A[(i - 1)]));
      i += 1;
    }
  }
  write(ch.query(0, N).ans, "\n");
  while (cpp_update(Q, "--"))
  {
    var k: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    read(k, x);
    ch.update(k, (k + 1), node(x, x, 0, x));
    write(ch.query(0, N).ans, "\n");
  }
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  return node(min(a.mi, (a.sum + b.mi)), max(a.mx, (a.sum + b.mx)), max(max(a.ans, b.ans), ((a.sum + b.mx) - a.mi)), (a.sum + b.sum));
}

func __cpp_lambda_2(a: dynamic, b: dynamic) -> dynamic
{
  return  ((b == err)) ? a : b;
}

func __cpp_lambda_3(a: dynamic, b: dynamic) -> dynamic
{
  return  ((b == err)) ? a : b;
}
