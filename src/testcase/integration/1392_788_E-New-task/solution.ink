// Translated from solution.cpp.

var iinf = (1e9 + 7);

var linf = (1 << 60);

var dinf = 1e60;

func scf(x: dynamic)
{
  var f = 0;
  x = 0;
  var c = getchar();
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

func scf(x: dynamic, y: dynamic)
{
  scf(x);
  return scf(y);
}

func scf(x: dynamic, y: dynamic, z: dynamic)
{
  scf(x);
  scf(y);
  return scf(z);
}

func scf(x: dynamic, y: dynamic, z: dynamic, w: dynamic)
{
  scf(x);
  scf(y);
  scf(z);
  return scf(w);
}

func mygetchar()
{
  var c = getchar();
  while (((c == cpp_char(" ")) || (c == cpp_char("\n"))))
  {
    c = getchar();
  }
  return c;
}

func chkmax(x: dynamic, y: dynamic)
{
  if ((y > x))
  {
    x = y;
  }
  return;
}

func chkmin(x: dynamic, y: dynamic)
{
  if ((y < x))
  {
    x = y;
  }
  return;
}

func main()
{
  TZL();
  RANK1();
  return 0;
}

var N = (1e5 + 100);

var mod = (1e9 + 7);

class node
{
  var f: dynamic = cpp_array(3, 3);
  func node()
  {
      memset((f), (0), cpp_sizeof(((f))));
    }
  func node(cntl: dynamic, cntr: dynamic)
  {
      memset((f), (0), cpp_sizeof(((f))));
      f[0][0] = cntl;
      f[2][2] = cntr;
      f[1][1] = 1;
      return;
    }
  func operator_add(a: dynamic)
  {
      var ret: dynamic;
      memcpy((ret.f), (f), cpp_sizeof(((f))));
      {
        var l = 0;
        var end = (3);
        while ((l < end))
        {
          {
            var r = 0;
            var end = (3);
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
        var l = 0;
        var end = (2);
        while ((l < end))
        {
          {
            var r = ((l + 1));
            var end = (2);
            while ((r <= end))
            {
              {
                var mid = (l);
                var end = ((r - 1));
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
  var n: dynamic;
  var root: dynamic;
  func B(cur: dynamic, l: dynamic, r: dynamic, a: dynamic, b: dynamic)
  {
      cur = cpp_new();
      if ((l == r))
      {
        cur->f = node(a[l], b[l]);
        return;
      }
      var mid = ((l + r) >> 1);
      B(cur->l, l, mid, a, b);
      B(cur->r, (mid + 1), r, a, b);
      return cur->pull();
    }
  func M(cur: dynamic, i: dynamic, x: dynamic, l: dynamic, r: dynamic)
  {
      if ((l == r))
      {
        cur->f = x;
        return;
      }
      var mid = ((l + r) >> 1);
      if ((i > mid))
      {
        M(cur->r, i, x, (mid + 1), r);
      } else
      {
        M(cur->l, i, x, l, mid);
      }
      return cur->pull();
    }
  func Q()
  {
      return root->f.f[0][2];
    }
  func B(n: dynamic, cntl: dynamic, cntr: dynamic)
  {
      B(root, 1, cpp_assign(n, "=", n), cntl, cntr);
      return;
    }
  func M(i: dynamic, x: dynamic)
  {
      return M(root, i, x, 1, n);
    }
  func out(cur: dynamic, l: dynamic, r: dynamic, dep: dynamic = 1)
  {
      if ((!cur))
      {
        return;
      }
      var mid = ((l + r) >> 1);
      out(cur->l, l, mid, (dep + 1));
      {
        var i = 0;
        var end = (dep);
        while ((i < end))
        {
          putchar(cpp_char("\t"));
          i += 1;
        }
      }
      printf("[%d, %d]:\n", l, r);
      {
        var i = 0;
        var end = (3);
        while ((i < end))
        {
          {
            var j = 0;
            var end = ((dep + 1));
            while ((j < end))
            {
              putchar(cpp_char("\t"));
              j += 1;
            }
          }
          {
            var j = 0;
            var end = (3);
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
        var i = 0;
        var end = (dep);
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
  func out()
  {
      return out(root, 1, n);
    }
}

var rt = cpp_array(N);

var n: dynamic;

var m: dynamic;

var ans: dynamic;

var a = cpp_array(N);

var cntl = cpp_array(N);

var cntr = cpp_array(N);

var apr = cpp_array(N);

var num = cpp_array(N);

func lowbit(i: dynamic)
{
  return (i & ((-i)));
}

func M(i: dynamic)
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

func Q(i: dynamic)
{
  var ret = 0;
  {
    while (i)
    {
      ret += num[i];
      i ^= lowbit(i);
    }
  }
  return ret;
}

func B()
{
  memset((num), (0), cpp_sizeof(((num))));
  return;
}

func TZL()
{
  var M: dynamic;
  M.clear();
  scf(n);
  {
    var i = (1);
    var end = (n);
    while ((i <= end))
    {
      scf(a[i]);
      M[a[i]];
      i += 1;
    }
  }
  {
    var it = M.begin();
    while ((it != M.end()))
    {
      it->second = (cpp_update(m, "++"));
      it += 1;
    }
  }
  {
    var i = (1);
    var end = (n);
    while ((i <= end))
    {
      a[i] = M[a[i]];
      i += 1;
    }
  }
  {
    var i = (1);
    var end = (n);
    while ((i <= end))
    {
      cntl[i] = BIT.Q(a[i]);
      BIT.M(a[i]);
      i += 1;
    }
  }
  BIT.B();
  {
    var i = (n);
    var end = (1);
    while ((i >= end))
    {
      cntr[i] = BIT.Q(a[i]);
      BIT.M(a[i]);
      i -= 1;
    }
  }
  {
    var i = (1);
    var end = (n);
    while ((i <= end))
    {
      apr[a[i]].push_back(i);
      i += 1;
    }
  }
  var foo = cpp_array(N);
  var bar = cpp_array(N);
  {
    var i = (1);
    var end = (m);
    while ((i <= end))
    {
      var pnt = 0;
      for (var x in apr[i])
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

func RANK1()
{
  var q_n: dynamic;
  scf(q_n);
  while (cpp_update(q_n, "--"))
  {
    var typ: dynamic;
    var i: dynamic;
    scf(typ, i);
    var x = a[i];
    var j = ((lower_bound(apr[x].begin(), apr[x].end(), i) - apr[x].begin()) + 1);
    (cpp_assign(ans, "+=", (mod - rt[x].Q()))) %= mod;
    var nw: dynamic;
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
