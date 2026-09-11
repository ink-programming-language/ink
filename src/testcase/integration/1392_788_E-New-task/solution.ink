// Translated from solution.cpp.

var iinf: dynamic = (1e9 + 7);

var linf: dynamic = (1 << 60);

var dinf: dynamic = 1e60;

func scf(x: dynamic) -> dynamic
{
  var f: dynamic = 0;
  x = 0;
  var c: dynamic = getchar();
  while (((((c < cpp_char("0")) || (c > cpp_char("9")))) && (c != cpp_char("-"))))
  {
    c = getchar();
  }
  if ((c == cpp_char("-")))
  {
    f = 1;
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = (((x * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  if (f)
  {
    x = (-x);
  }
  return;
}

func scf(x: dynamic, y: dynamic) -> dynamic
{
  scf(x);
  return scf(y);
}

func scf(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  scf(x);
  scf(y);
  return scf(z);
}

func scf(x: dynamic, y: dynamic, z: dynamic, w: dynamic) -> dynamic
{
  scf(x);
  scf(y);
  scf(z);
  return scf(w);
}

func mygetchar() -> dynamic
{
  var c: dynamic = getchar();
  while (((c == cpp_char(" ")) || (c == cpp_char("\n"))))
  {
    c = getchar();
  }
  return c;
}

func chkmax(x: dynamic, y: dynamic) -> dynamic
{
  if ((y > x))
  {
    x = y;
  }
  return;
}

func chkmin(x: dynamic, y: dynamic) -> dynamic
{
  if ((y < x))
  {
    x = y;
  }
  return;
}

func main() -> dynamic
{
  TZL();
  RANK1();
  return 0;
}

var N: dynamic = (1e5 + 100);

var mod: dynamic = (1e9 + 7);

class node
{
  var f: dynamic = cpp_array(3, 3);
  func node() -> dynamic
  {
      memset((f), (0), cpp_sizeof(((f))));
    }
  func node(cntl: dynamic, cntr: dynamic) -> dynamic
  {
      memset((f), (0), cpp_sizeof(((f))));
      f[0][0] = cntl;
      f[2][2] = cntr;
      f[1][1] = 1;
      return;
    }
  func operator_add(a: dynamic) -> dynamic
  {
      var ret: dynamic = cpp_uninitialized();
      memcpy((ret.f), (f), cpp_sizeof(((f))));
      {
        var l: dynamic = 0;
        var end: dynamic = (3);
        while ((l < end))
        {
          {
            var r: dynamic = 0;
            var end: dynamic = (3);
            while ((r < end))
            {
              (cpp_assign(ret.f[l][r], "+=", a.f[l][r])) %= mod;
              r += 1;
            }
          }
          l += 1;
        }
      }
      {
        var l: dynamic = 0;
        var end: dynamic = (2);
        while ((l < end))
        {
          {
            var r: dynamic = ((l + 1));
            var end: dynamic = (2);
            while ((r <= end))
            {
              {
                var mid: dynamic = (l);
                var end: dynamic = ((r - 1));
                while ((mid <= end))
                {
                  ret.f[l][r] = ((((1 * ret.f[l][r]) + ((1 * f[l][mid]) * a.f[(mid + 1)][r]))) % mod);
                  mid += 1;
                }
              }
              r += 1;
            }
          }
          l += 1;
        }
      }
      return ret;
    }
}

class seg
{
  var n: dynamic = cpp_uninitialized();
  var root: dynamic = cpp_uninitialized();
  func B(cur: dynamic, l: dynamic, r: dynamic, a: dynamic, b: dynamic) -> dynamic
  {
      cur = cpp_new();
      if ((l == r))
      {
        cur->f = node(a[l], b[l]);
        return;
      }
      var mid: dynamic = ((l + r) >> 1);
      B(cur->l, l, mid, a, b);
      B(cur->r, (mid + 1), r, a, b);
      return cur->pull();
    }
  func M(cur: dynamic, i: dynamic, x: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      if ((l == r))
      {
        cur->f = x;
        return;
      }
      var mid: dynamic = ((l + r) >> 1);
      if ((i > mid))
      {
        M(cur->r, i, x, (mid + 1), r);
      } else
      {
        M(cur->l, i, x, l, mid);
      }
      return cur->pull();
    }
  func Q() -> dynamic
  {
      return root->f.f[0][2];
    }
  func B(n: dynamic, cntl: dynamic, cntr: dynamic) -> dynamic
  {
      B(root, 1, cpp_assign(n, "=", n), cntl, cntr);
      return;
    }
  func M(i: dynamic, x: dynamic) -> dynamic
  {
      return M(root, i, x, 1, n);
    }
  func out(cur: dynamic, l: dynamic, r: dynamic, dep: dynamic = 1) -> dynamic
  {
      if ((!cur))
      {
        return;
      }
      var mid: dynamic = ((l + r) >> 1);
      out(cur->l, l, mid, (dep + 1));
      {
        var i: dynamic = 0;
        var end: dynamic = (dep);
        while ((i < end))
        {
          putchar(cpp_char("\t"));
          i += 1;
        }
      }
      printf("[%d, %d]:\n", l, r);
      {
        var i: dynamic = 0;
        var end: dynamic = (3);
        while ((i < end))
        {
          {
            var j: dynamic = 0;
            var end: dynamic = ((dep + 1));
            while ((j < end))
            {
              putchar(cpp_char("\t"));
              j += 1;
            }
          }
          {
            var j: dynamic = 0;
            var end: dynamic = (3);
            while ((j < end))
            {
              printf("%d ", cur->f.f[i][j]);
              j += 1;
            }
          }
          putchar(cpp_char("\n"));
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        var end: dynamic = (dep);
        while ((i < end))
        {
          putchar(cpp_char("\t"));
          i += 1;
        }
      }
      puts("============");
      out(cur->r, (mid + 1), r, (dep + 1));
      return;
    }
  func out() -> dynamic
  {
      return out(root, 1, n);
    }
}

var rt: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var cntl: dynamic = cpp_array(N);

var cntr: dynamic = cpp_array(N);

var apr: dynamic = cpp_array(N);

var num: dynamic = cpp_array(N);

func lowbit(i: dynamic) -> dynamic
{
  return (i & ((-i)));
}

func M(i: dynamic) -> dynamic
{
  {
    while ((i <= m))
    {
      num[i] += 1;
      i += lowbit(i);
    }
  }
  return;
}

func Q(i: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  {
    while (i)
    {
      ret += num[i];
      i ^= lowbit(i);
    }
  }
  return ret;
}

func B() -> dynamic
{
  memset((num), (0), cpp_sizeof(((num))));
  return;
}

func TZL() -> dynamic
{
  var M: dynamic = cpp_uninitialized();
  M.clear();
  scf(n);
  {
    var i: dynamic = (1);
    var end: dynamic = (n);
    while ((i <= end))
    {
      scf(a[i]);
      M[a[i]];
      i += 1;
    }
  }
  {
    var it: dynamic = M.begin();
    while ((it != M.end()))
    {
      it->second = (cpp_update(m, "++"));
      it += 1;
    }
  }
  {
    var i: dynamic = (1);
    var end: dynamic = (n);
    while ((i <= end))
    {
      a[i] = M[a[i]];
      i += 1;
    }
  }
  {
    var i: dynamic = (1);
    var end: dynamic = (n);
    while ((i <= end))
    {
      cntl[i] = BIT.Q(a[i]);
      BIT.M(a[i]);
      i += 1;
    }
  }
  BIT.B();
  {
    var i: dynamic = (n);
    var end: dynamic = (1);
    while ((i >= end))
    {
      cntr[i] = BIT.Q(a[i]);
      BIT.M(a[i]);
      i -= 1;
    }
  }
  {
    var i: dynamic = (1);
    var end: dynamic = (n);
    while ((i <= end))
    {
      apr[a[i]].push_back(i);
      i += 1;
    }
  }
  var foo: dynamic = cpp_array(N);
  var bar: dynamic = cpp_array(N);
  {
    var i: dynamic = (1);
    var end: dynamic = (m);
    while ((i <= end))
    {
      var pnt: dynamic = 0;
      for (var x: dynamic in apr[i])
      {
        pnt += 1;
        foo[pnt] = cntl[x];
        bar[pnt] = cntr[x];
      }
      rt[i].B(pnt, foo, bar);
      (cpp_assign(ans, "+=", rt[i].Q())) %= mod;
      i += 1;
    }
  }
  return;
}

func RANK1() -> dynamic
{
  var q_n: dynamic = cpp_uninitialized();
  scf(q_n);
  while (cpp_update(q_n, "--"))
  {
    var typ: dynamic = cpp_uninitialized();
    var i: dynamic = cpp_uninitialized();
    scf(typ, i);
    var x: dynamic = a[i];
    var j: dynamic = ((lower_bound(apr[x].begin(), apr[x].end(), i) - apr[x].begin()) + 1);
    (cpp_assign(ans, "+=", (mod - rt[x].Q()))) %= mod;
    var nw: dynamic = cpp_uninitialized();
    if ((typ == 2))
    {
      nw = node(cntl[i], cntr[i]);
    }
    rt[x].M(j, nw);
    (cpp_assign(ans, "+=", rt[x].Q())) %= mod;
    printf("%d\n", ans);
  }
  return;
}
