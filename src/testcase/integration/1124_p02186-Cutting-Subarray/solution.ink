// Translated from solution.cpp.

var F = cpp_expression("#incl");

var S = cpp_expression("#inclu");

var MOD = 1000000007;

func operator_shift_right(i: dynamic, A: dynamic)
{
  ((i >> A.F) >> A.S);
  return i;
}

func operator_shift_right(i: dynamic, A: dynamic)
{
  for (var I in A)
  {
    (i >> I);
  }
  return i;
}

func operator_shift_left(o: dynamic, A: dynamic)
{
  (((o << A.F) << " ") << A.S);
  return o;
}

func operator_shift_left(o: dynamic, A: dynamic)
{
  var i = A.size();
  for (var I in A)
  {
    ((o << I) << (if (cpp_update(i, "--")) " " else ""));
  }
  return o;
}

class SegmentTree
{
  var n: dynamic;
  var height: dynamic;
  var f: dynamic;
  var g: dynamic;
  var h: dynamic;
  var ti: dynamic;
  var ei: dynamic;
  var dat: dynamic;
  var laz: dynamic;
  func SegmentTree(f: dynamic, g: dynamic, h: dynamic, ti: dynamic, ei: dynamic)
  {
      this->f = cpp_construct(f);
      this->g = cpp_construct(g);
      this->h = cpp_construct(h);
      this->ti = cpp_construct(ti);
      this->ei = cpp_construct(ei);
    }
  func init(n: dynamic)
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
  func build(v: dynamic)
  {
      var n = v.size();
      init(n);
      {
        var i = 0;
        while ((i < n))
        {
          dat[(n + i)] = v[i];
          i += 1;
        }
      }
      {
        var i = (n - 1);
        while (i)
        {
          dat[i] = f(dat[(((i << 1)) | 0)], dat[(((i << 1)) | 1)]);
          i -= 1;
        }
      }
    }
  func reflect(k: dynamic)
  {
      return if ((laz[k] == ei)) dat[k] else g(dat[k], laz[k]);
    }
  func eval(k: dynamic)
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
  func thrust(k: dynamic)
  {
      {
        var i = height;
        while (i)
        {
          eval((k >> i));
          i -= 1;
        }
      }
    }
  func recalc(k: dynamic)
  {
      while (cpp_assign(k, ">>=", 1))
      {
        dat[k] = f(reflect((((k << 1)) | 0)), reflect((((k << 1)) | 1)));
      }
    }
  func update(a: dynamic, b: dynamic, x: dynamic)
  {
      thrust(cpp_assign(a, "+=", n));
      thrust(cpp_assign(b, "+=", (n - 1)));
      {
        var l = a;
        var r = (b + 1);
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
  func set_val(a: dynamic, x: dynamic)
  {
      thrust(cpp_assign(a, "+=", n));
      dat[a] = x;
      laz[a] = ei;
      recalc(a);
    }
  func query(a: dynamic, b: dynamic)
  {
      thrust(cpp_assign(a, "+=", n));
      thrust(cpp_assign(b, "+=", (n - 1)));
      var vl = ti;
      var vr = ti;
      {
        var l = a;
        var r = (b + 1);
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
  func find(st: dynamic, check: dynamic, acc: dynamic, k: dynamic, l: dynamic, r: dynamic)
  {
      if (((l + 1) == r))
      {
        acc = f(acc, reflect(k));
        return if (check(acc)) (k - n) else -1;
      }
      eval(k);
      var m = (((l + r)) >> 1);
      if ((m <= st))
      {
        return find(st, check, acc, (((k << 1)) | 1), m, r);
      }
      if (((st <= l) && (!check(f(acc, dat[k])))))
      {
        acc = f(acc, dat[k]);
        return -1;
      }
      var vl = find(st, check, acc, (((k << 1)) | 0), l, m);
      if ((~vl))
      {
        return vl;
      }
      return find(st, check, acc, (((k << 1)) | 1), m, r);
    }
  func find(st: dynamic, check: dynamic)
  {
      var acc = ti;
      return find(st, check, acc, 1, 0, n);
    }
}

class node
{
  var mi: dynamic;
  var mx: dynamic;
  var ans: dynamic;
  var sum: dynamic;
  func node(a: dynamic = 0, b: dynamic = 0, c: dynamic = 0, d: dynamic = 0)
  {
      this->mi = cpp_construct(a);
      this->mx = cpp_construct(b);
      this->ans = cpp_construct(c);
      this->sum = cpp_construct(d);
    }
  func operator_equal(A: dynamic)
  {
      return ((((A.mi == mi) && (A.mx == mx)) && (A.ans == ans)) && (A.sum == sum));
    }
}

var uku = (-1 * MOD);

var err = node(MOD, uku, 0, 0);

func main()
{
  var f = __cpp_lambda_1;
  var g = __cpp_lambda_2;
  var h = __cpp_lambda_3;
  var N: dynamic;
  var Q: dynamic;
  read(N, Q);
  N += 1;
  ch.build(vector(N, node()));
  var A = cpp_construct((N - 1));
  read(A);
  {
    var i = 1;
    while ((i <= N))
    {
      ch.update(i, (i + 1), node(A[(i - 1)], A[(i - 1)], 0, A[(i - 1)]));
      i += 1;
    }
  }
  write(ch.query(0, N).ans, "\n");
  while (cpp_update(Q, "--"))
  {
    var k: dynamic;
    var x: dynamic;
    read(k, x);
    ch.update(k, (k + 1), node(x, x, 0, x));
    write(ch.query(0, N).ans, "\n");
  }
  return 0;
}

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return node(min(a.mi, (a.sum + b.mi)), max(a.mx, (a.sum + b.mx)), max(max(a.ans, b.ans), ((a.sum + b.mx) - a.mi)), (a.sum + b.sum));
}

func __cpp_lambda_2(a: dynamic, b: dynamic)
{
  return if ((b == err)) a else b;
}

func __cpp_lambda_3(a: dynamic, b: dynamic)
{
  return if ((b == err)) a else b;
}
