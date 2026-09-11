// Translated from solution.cpp.

func max(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  return max(a, max(b, c));
}

class Node
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var ll: dynamic = cpp_uninitialized();
  var lr: dynamic = cpp_uninitialized();
  var rl: dynamic = cpp_uninitialized();
  var rr: dynamic = cpp_uninitialized();
  var hi: dynamic = cpp_uninitialized();
  var lhi: dynamic = cpp_uninitialized();
  var rhi: dynamic = cpp_uninitialized();
  var nhi: dynamic = cpp_uninitialized();
  var nlhi: dynamic = cpp_uninitialized();
  var nrhi: dynamic = cpp_uninitialized();
  var mark: dynamic = cpp_uninitialized();
}

var s: dynamic = cpp_array((500005 * 4));

func read(x: dynamic) -> dynamic
{
  x = 0;
  var c: dynamic = getchar();
  var sig: dynamic = 1;
  {
    while ((!isdigit(c)))
    {
      if ((c == cpp_char("-")))
      {
        sig = -1;
      }
      c = getchar();
    }
  }
  {
    while (isdigit(c))
    {
      x = (((((x << 3)) + ((x << 1))) + c) - cpp_char("0"));
      c = getchar();
    }
  }
  x *= sig;
}

func combine(l: dynamic, r: dynamic) -> dynamic
{
  var ans: dynamic = cpp_uninitialized();
  ans.l = l.l;
  ans.r = r.r;
  ans.ll = l.ll;
  if ((l.ll == ((l.r - l.l) + 1)))
  {
    ans.ll += r.ll;
  }
  ans.lr = l.lr;
  if ((l.lr == ((l.r - l.l) + 1)))
  {
    ans.lr += r.lr;
  }
  ans.rl = r.rl;
  if ((r.rl == ((r.r - r.l) + 1)))
  {
    ans.rl += l.rl;
  }
  ans.rr = r.rr;
  if ((r.rr == ((r.r - r.l) + 1)))
  {
    ans.rr += l.rr;
  }
  var lhi: dynamic = l.lhi;
  if ((lhi == ((l.r - l.l) + 1)))
  {
    lhi += r.ll;
  }
  if (((lhi == 0) && (l.lr == ((l.r - l.l) + 1))))
  {
    if ((r.ll > 0))
    {
      lhi = (l.lr + r.ll);
    }
    if ((r.lhi > 0))
    {
      lhi = (l.lr + r.lhi);
    }
  }
  ans.lhi = lhi;
  var rhi: dynamic = r.rhi;
  if ((rhi == ((r.r - r.l) + 1)))
  {
    rhi += l.rr;
  }
  if (((rhi == 0) && (r.rl == ((r.r - r.l) + 1))))
  {
    if ((l.rr > 0))
    {
      rhi = (r.rl + l.rr);
    }
    if ((l.rhi > 0))
    {
      rhi = (r.rl + l.rhi);
    }
  }
  ans.rhi = rhi;
  var mlr: dynamic =  ((l.rhi > 0)) ? (l.rhi + r.ll) : 0;
  var mrl: dynamic =  ((r.lhi > 0)) ? (r.lhi + l.rr) : 0;
  var m: dynamic =  (((l.rr > 0) && (r.ll > 0))) ? (l.rr + r.ll) : 0;
  ans.hi = max(max(ans.lhi, ans.rhi), max(mlr, mrl, m), max(l.hi, r.hi));
  var nlhi: dynamic = l.nlhi;
  if ((nlhi == ((l.r - l.l) + 1)))
  {
    nlhi += r.lr;
  }
  if (((nlhi == 0) && (l.ll == ((l.r - l.l) + 1))))
  {
    if ((r.lr > 0))
    {
      nlhi = (l.ll + r.lr);
    }
    if ((r.nlhi > 0))
    {
      nlhi = (l.ll + r.nlhi);
    }
  }
  ans.nlhi = nlhi;
  var nrhi: dynamic = r.nrhi;
  if ((nrhi == ((r.r - r.l) + 1)))
  {
    nrhi += l.rl;
  }
  if (((nrhi == 0) && (r.rr == ((r.r - r.l) + 1))))
  {
    if ((l.rl > 0))
    {
      nrhi = (r.rr + l.rl);
    }
    if ((l.nrhi > 0))
    {
      nrhi = (r.rr + l.nrhi);
    }
  }
  ans.nrhi = nrhi;
  var nmlr: dynamic =  ((l.nrhi > 0)) ? (l.nrhi + r.lr) : 0;
  var nmrl: dynamic =  ((r.nlhi > 0)) ? (r.nlhi + l.rl) : 0;
  var nm: dynamic =  (((l.rl > 0) && (r.lr > 0))) ? (l.rl + r.lr) : 0;
  ans.nhi = max(max(ans.nlhi, ans.nrhi), max(nmlr, nmrl, nm), max(l.nhi, r.nhi));
  return ans;
}

func calc(idx: dynamic) -> dynamic
{
  s[idx] = combine(s[((idx << 1))], s[(((idx << 1) | 1))]);
}

func push_down(idx: dynamic) -> dynamic
{
  if ((!s[idx].mark))
  {
    return;
  }
  {
    var i: dynamic = ((idx << 1));
    while ((i <= (((idx << 1) | 1))))
    {
      flip(s[i]);
      i += 1;
    }
  }
  s[idx].mark = false;
}

func build(idx: dynamic, l: dynamic, r: dynamic, ss: dynamic) -> dynamic
{
  s[idx].l = l;
  s[idx].r = r;
  if ((l == r))
  {
    if ((ss[(l - 1)] == cpp_char("<")))
    {
      s[idx].ll = 1;
      s[idx].lr = 0;
      s[idx].rl = 1;
      s[idx].rr = 0;
    } else
    {
      s[idx].ll = 0;
      s[idx].lr = 1;
      s[idx].rl = 0;
      s[idx].rr = 1;
    }
    return;
  }
  var mid: dynamic = (((l + r)) >> 1);
  build(((idx << 1)), l, mid, ss);
  build((((idx << 1) | 1)), (mid + 1), r, ss);
  calc(idx);
}

func flip(raw: dynamic) -> dynamic
{
  raw.mark = (!raw.mark);
  swap(raw.lr, raw.ll);
  swap(raw.rl, raw.rr);
  swap(raw.lhi, raw.nlhi);
  swap(raw.rhi, raw.nrhi);
  swap(raw.hi, raw.nhi);
}

func flip(idx: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if (((s[idx].l >= l) && (s[idx].r <= r)))
  {
    flip(s[idx]);
    return;
  }
  push_down(idx);
  var mid: dynamic = (((s[idx].l + s[idx].r)) >> 1);
  if ((l <= mid))
  {
    flip(((idx << 1)), l, r);
  }
  if ((mid < r))
  {
    flip((((idx << 1) | 1)), l, r);
  }
  calc(idx);
}

func query(idx: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if (((s[idx].l >= l) && (s[idx].r <= r)))
  {
    return s[idx];
  }
  push_down(idx);
  var ans: dynamic = cpp_uninitialized();
  var mid: dynamic = (((s[idx].l + s[idx].r)) >> 1);
  if ((l <= mid))
  {
    ans = query(((idx << 1)), l, r);
  }
  if ((mid < r))
  {
    var right: dynamic = query((((idx << 1) | 1)), l, r);
    if ((ans.l == 0))
    {
      ans = right;
    } else
    {
      ans = combine(ans, right);
    }
  }
  return ans;
}

class Solution
{
  func solve() -> dynamic
  {
      var n: dynamic = cpp_uninitialized();
      var q: dynamic = cpp_uninitialized();
      read(n);
      read(q);
      var ss: dynamic = cpp_uninitialized();
      read(ss);
      build(1, 1, n, ss);
      {
        var i: dynamic = 0;
        while ((i < q))
        {
          var l: dynamic = cpp_uninitialized();
          var r: dynamic = cpp_uninitialized();
          read(l);
          read(r);
          flip(1, l, r);
          var q: dynamic = query(1, l, r);
          var ans: dynamic = max(q.hi, max(q.ll, q.rr));
          write(ans, "\n");
          i += 1;
        }
      }
    }
}

func main() -> dynamic
{
  var solution: dynamic = Solution();
  solution.solve();
}
