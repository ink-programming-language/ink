// Translated from solution.cpp.

func get()
{
  var ch: dynamic;
  while (cpp_comma(cpp_assign(ch, "=", getchar()), ((((ch < cpp_char("0")) || (ch > cpp_char("9")))) && (ch != cpp_char("-")))))
  {
  }
  if ((ch == cpp_char("-")))
  {
    var s = 0;
    while (cpp_comma(cpp_assign(ch, "=", getchar()), ((ch >= cpp_char("0")) && (ch <= cpp_char("9")))))
    {
      s = (((s * 10) + ch) - cpp_char("0"));
    }
    return (-s);
  }
  var s = (ch - cpp_char("0"));
  while (cpp_comma(cpp_assign(ch, "=", getchar()), ((ch >= cpp_char("0")) && (ch <= cpp_char("9")))))
  {
    s = (((s * 10) + ch) - cpp_char("0"));
  }
  return s;
}

var N = 65;

var mo = (1e9 + 7);

var MAXN = 2e6;

var js = cpp_array(100005);

var n: dynamic;

var k: dynamic;

var fa = cpp_array(N);

class node
{
  var x: dynamic;
  var y: dynamic;
  var v: dynamic;
}

var a = cpp_array(N);

var p = cpp_array(N);

var mi = cpp_array(70);

var m: dynamic;

var idx: dynamic;

var idy: dynamic;

var kx: dynamic;

var ky: dynamic;

var c = cpp_array(N);

var tmp = cpp_array(N);

var s = cpp_array(N);

var suf = cpp_array(N);

class zt
{
  var u: dynamic;
  var cnt: dynamic;
  var val: dynamic;
}

var que = cpp_array((MAXN + 5));

var id = cpp_array(N);

var vis = cpp_array(N);

var kth = cpp_array(N);

var td = cpp_array(N);

func getfather(x: dynamic)
{
  return if ((fa[x] == x)) x else cpp_assign(fa[x], "=", getfather(fa[x]));
}

func cmp(a: dynamic, b: dynamic)
{
  return (kth[a.x] < kth[b.x]);
}

func dec(x: dynamic, y: dynamic)
{
  return if ((x < y)) ((x - y) + mo) else (x + y);
}

func add(x: dynamic, y: dynamic)
{
  return if (((x + y) >= mo)) ((x + y) - mo) else (x + y);
}

var num = cpp_array(N);

func getcnt(v: dynamic)
{
  var ret = 0;
  {
    while (v)
    {
      ret += 1;
      v -= (v & (-v));
    }
  }
  return ret;
}

func main()
{
  n = get();
  k = get();
  {
    var i = 1;
    while ((i <= k))
    {
      a[i].x = get();
      a[i].y = get();
      a[i].v = (get() - 1);
      i += 1;
    }
  }
  srand(20010419);
  random_shuffle((a + 1), ((a + 1) + k));
  {
    var i = 1;
    while ((i <= k))
    {
      fa[i] = i;
      i += 1;
    }
  }
  js[0] = 1;
  {
    var i = 1;
    while ((i <= 100000))
    {
      js[i] = ((js[(i - 1)] * i) % mo);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= k))
    {
      {
        var j = (i + 1);
        while ((j <= k))
        {
          if (((a[i].x == a[j].x) || (a[i].y == a[j].y)))
          {
            var fx = getfather(i);
            var fy = getfather(j);
            fa[fy] = fx;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  mi[0] = 1;
  {
    var i = 1;
    while ((i <= k))
    {
      mi[i] = (mi[(i - 1)] * 2);
      i += 1;
    }
  }
  c[0] = 1;
  {
    var tp = 1;
    while ((tp <= k))
    {
      if ((getfather(tp) == tp))
      {
        m = 0;
        idx.clear();
        idy.clear();
        kx = cpp_assign(ky, "=", 0);
        {
          var i = 1;
          while ((i <= k))
          {
            if ((getfather(i) == tp))
            {
              if ((!idx[a[i].x]))
              {
                idx[a[i].x] = cpp_update(kx, "++");
              }
              if ((!idy[a[i].y]))
              {
                idy[a[i].y] = cpp_update(ky, "++");
              }
              p[cpp_update(m, "++")].x = idx[a[i].x];
              p[m].y = idy[a[i].y];
              p[m].v = a[i].v;
            }
            i += 1;
          }
        }
        if ((kx < ky))
        {
          swap(kx, ky);
          {
            var i = 1;
            while ((i <= m))
            {
              swap(p[i].x, p[i].y);
              i += 1;
            }
          }
        }
        {
          var i = 1;
          while ((i <= kx))
          {
            vis[cpp_assign(num[i], "=", i)] = cpp_assign(td[i], "=", 0);
            i += 1;
          }
        }
        {
          var i = 1;
          while ((i <= m))
          {
            td[p[i].x] |= mi[p[i].y];
            i += 1;
          }
        }
        var now = 0;
        {
          var i = kx;
          while ((i >= 1))
          {
            var key = 0;
            var cnt = 0;
            var tp = 0;
            {
              var j = 1;
              while ((j <= kx))
              {
                if ((!vis[j]))
                {
                  if (((!key) || (cnt > getcnt((now | td[j])))))
                  {
                    tp = (now | td[j]);
                    cnt = getcnt(tp);
                    key = j;
                  }
                }
                j += 1;
              }
            }
            vis[key] = 1;
            now = tp;
            kth[cpp_assign(num[i], "=", key)] = i;
            i -= 1;
          }
        }
        sort((p + 1), ((p + 1) + m), cmp);
        suf[(m + 1)] = 0;
        {
          var i = m;
          while ((i >= 1))
          {
            suf[i] = (suf[(i + 1)] | mi[p[i].y]);
            i -= 1;
          }
        }
        var he = 0;
        var ta = 1;
        que[1].cnt = 0;
        que[1].val = 1;
        que[1].u = 0;
        var w = 1;
        {
          var i = 1;
          while ((i <= kx))
          {
            {
              var j = 0;
              while ((j <= k))
              {
                id[j].clear();
                j += 1;
              }
            }
            var qw = w;
            while (((w <= m) && (p[w].x == num[i])))
            {
              w += 1;
            }
            var qt = ta;
            while ((he < qt))
            {
              he += 1;
              var nu = que[he].u;
              var nv = que[he].val;
              var cnt = que[he].cnt;
              var to: dynamic;
              if (id[cnt][(nu & suf[w])])
              {
                to = id[cnt][(nu & suf[w])];
              } else
              {
                to = cpp_update(ta, "++");
                que[to].cnt = cnt;
                que[to].u = nu;
                que[to].val = 0;
                id[cnt][(nu & suf[w])] = to;
              }
              que[to].val = add(que[to].val, nv);
              {
                var x = qw;
                while ((x <= (w - 1)))
                {
                  if ((((nu & mi[p[x].y])) == 0))
                  {
                    var u = (((nu ^ mi[p[x].y])) & suf[w]);
                    var to: dynamic;
                    if (id[(cnt + 1)][u])
                    {
                      to = id[(cnt + 1)][u];
                    } else
                    {
                      to = cpp_update(ta, "++");
                      que[to].cnt = (cnt + 1);
                      que[to].u = (nu ^ mi[p[x].y]);
                      que[to].val = 0;
                      id[(cnt + 1)][u] = to;
                    }
                    que[to].val = add(que[to].val, ((nv * p[x].v) % mo));
                  }
                  x += 1;
                }
              }
            }
            i += 1;
          }
        }
        {
          var i = 0;
          while ((i <= k))
          {
            tmp[i] = cpp_assign(s[i], "=", 0);
            i += 1;
          }
        }
        {
          while ((he < ta))
          {
            he += 1;
            s[que[he].cnt] = add(s[que[he].cnt], que[he].val);
          }
        }
        {
          var i = 0;
          while ((i <= k))
          {
            {
              var j = 0;
              while ((j <= (k - i)))
              {
                tmp[(i + j)] = add(tmp[(i + j)], ((c[i] * s[j]) % mo));
                j += 1;
              }
            }
            i += 1;
          }
        }
        {
          var i = 0;
          while ((i <= k))
          {
            c[i] = tmp[i];
            i += 1;
          }
        }
      }
      tp += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i <= k))
    {
      ans = add(ans, ((c[i] * js[(n - i)]) % mo));
      i += 1;
    }
  }
  printf("%I64d\n", ans);
  return 0;
}
