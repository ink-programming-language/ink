// Translated from solution.cpp.

func get() -> dynamic
{
  var ch: dynamic = cpp_uninitialized();
  while (cpp_comma(cpp_assign(ch, "=", getchar()), ((((ch < cpp_char("0")) || (ch > cpp_char("9")))) && (ch != cpp_char("-")))))
  {
  }
  if ((ch == cpp_char("-")))
  {
    var s: dynamic = 0;
    while (cpp_comma(cpp_assign(ch, "=", getchar()), ((ch >= cpp_char("0")) && (ch <= cpp_char("9")))))
    {
      s = (((s * 10) + ch) - cpp_char("0"));
    }
    return (-s);
  }
  var s: dynamic = (ch - cpp_char("0"));
  while (cpp_comma(cpp_assign(ch, "=", getchar()), ((ch >= cpp_char("0")) && (ch <= cpp_char("9")))))
  {
    s = (((s * 10) + ch) - cpp_char("0"));
  }
  return s;
}

var N: dynamic = 65;

var mo: dynamic = (1e9 + 7);

var MAXN: dynamic = 2e6;

var js: dynamic = cpp_array(100005);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var fa: dynamic = cpp_array(N);

class node
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
}

var a: dynamic = cpp_array(N);

var p: dynamic = cpp_array(N);

var mi: dynamic = cpp_array(70);

var m: dynamic = cpp_uninitialized();

var idx: dynamic = cpp_uninitialized();

var idy: dynamic = cpp_uninitialized();

var kx: dynamic = cpp_uninitialized();

var ky: dynamic = cpp_uninitialized();

var c: dynamic = cpp_array(N);

var tmp: dynamic = cpp_array(N);

var s: dynamic = cpp_array(N);

var suf: dynamic = cpp_array(N);

class zt
{
  var u: dynamic = cpp_uninitialized();
  var cnt: dynamic = cpp_uninitialized();
  var val: dynamic = cpp_uninitialized();
}

var que: dynamic = cpp_array((MAXN + 5));

var id: dynamic = cpp_array(N);

var vis: dynamic = cpp_array(N);

var kth: dynamic = cpp_array(N);

var td: dynamic = cpp_array(N);

func getfather(x: dynamic) -> dynamic
{
  return  ((fa[x] == x)) ? x : cpp_assign(fa[x], "=", getfather(fa[x]));
}

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (kth[a.x] < kth[b.x]);
}

func dec(x: dynamic, y: dynamic) -> dynamic
{
  return  ((x < y)) ? ((x - y) + mo) : (x + y);
}

func add(x: dynamic, y: dynamic) -> dynamic
{
  return  (((x + y) >= mo)) ? ((x + y) - mo) : (x + y);
}

var num: dynamic = cpp_array(N);

func getcnt(v: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  {
    while (v)
    {
      ret += 1;
      v -= (v & (-v));
    }
  }
  return ret;
}

func main() -> dynamic
{
  n = get();
  k = get();
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= k))
    {
      fa[i] = i;
      i += 1;
    }
  }
  js[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= 100000))
    {
      js[i] = ((js[(i - 1)] * i) % mo);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      {
        var j: dynamic = (i + 1);
        while ((j <= k))
        {
          if (((a[i].x == a[j].x) || (a[i].y == a[j].y)))
          {
            var fx: dynamic = getfather(i);
            var fy: dynamic = getfather(j);
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
    var i: dynamic = 1;
    while ((i <= k))
    {
      mi[i] = (mi[(i - 1)] * 2);
      i += 1;
    }
  }
  c[0] = 1;
  {
    var tp: dynamic = 1;
    while ((tp <= k))
    {
      if ((getfather(tp) == tp))
      {
        m = 0;
        idx.clear();
        idy.clear();
        kx = cpp_assign(ky, "=", 0);
        {
          var i: dynamic = 1;
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
            var i: dynamic = 1;
            while ((i <= m))
            {
              swap(p[i].x, p[i].y);
              i += 1;
            }
          }
        }
        {
          var i: dynamic = 1;
          while ((i <= kx))
          {
            vis[cpp_assign(num[i], "=", i)] = cpp_assign(td[i], "=", 0);
            i += 1;
          }
        }
        {
          var i: dynamic = 1;
          while ((i <= m))
          {
            td[p[i].x] |= mi[p[i].y];
            i += 1;
          }
        }
        var now: dynamic = 0;
        {
          var i: dynamic = kx;
          while ((i >= 1))
          {
            var key: dynamic = 0;
            var cnt: dynamic = 0;
            var tp: dynamic = 0;
            {
              var j: dynamic = 1;
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
          var i: dynamic = m;
          while ((i >= 1))
          {
            suf[i] = (suf[(i + 1)] | mi[p[i].y]);
            i -= 1;
          }
        }
        var he: dynamic = 0;
        var ta: dynamic = 1;
        que[1].cnt = 0;
        que[1].val = 1;
        que[1].u = 0;
        var w: dynamic = 1;
        {
          var i: dynamic = 1;
          while ((i <= kx))
          {
            {
              var j: dynamic = 0;
              while ((j <= k))
              {
                id[j].clear();
                j += 1;
              }
            }
            var qw: dynamic = w;
            while (((w <= m) && (p[w].x == num[i])))
            {
              w += 1;
            }
            var qt: dynamic = ta;
            while ((he < qt))
            {
              he += 1;
              var nu: dynamic = que[he].u;
              var nv: dynamic = que[he].val;
              var cnt: dynamic = que[he].cnt;
              var to: dynamic = cpp_uninitialized();
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
                var x: dynamic = qw;
                while ((x <= (w - 1)))
                {
                  if ((((nu & mi[p[x].y])) == 0))
                  {
                    var u: dynamic = (((nu ^ mi[p[x].y])) & suf[w]);
                    var to: dynamic = cpp_uninitialized();
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
          var i: dynamic = 0;
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
          var i: dynamic = 0;
          while ((i <= k))
          {
            {
              var j: dynamic = 0;
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
          var i: dynamic = 0;
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
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= k))
    {
      ans = add(ans, ((c[i] * js[(n - i)]) % mo));
      i += 1;
    }
  }
  printf("%I64d\n", ans);
  return 0;
}
