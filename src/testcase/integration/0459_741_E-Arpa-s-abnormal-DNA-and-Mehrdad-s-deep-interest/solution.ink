// Translated from solution.cpp.

var MAXN = 200007;

var MAXQ = 100003;

var LOGN = 18;

var ALPHA_SZ = 29;

var ALPHA_OFFSET = cpp_char("_");

var SOLANUM_TUBEROSUM = 128;

var s: dynamic;

var n: dynamic;

var sa = cpp_array(MAXN);

var rk = cpp_array(MAXN);

var lcp = cpp_array(MAXN);

var nw = cpp_array(MAXN);

var st = cpp_array(LOGN, MAXN);

func bucket_sort(k: dynamic)
{
  var sz = max(ALPHA_SZ, n);
  var ct = cpp_array(MAXN);
  {
    var i = 0;
    while ((i < sz))
    {
      ct[i] = 0;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      ct[rk[(sa[i] + k)]] += 1;
      i += 1;
    }
  }
  var sum = 0;
  var t: dynamic;
  {
    var i = 0;
    while ((i < sz))
    {
      t = ct[i];
      ct[i] = sum;
      sum += t;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      nw[cpp_update(ct[rk[(sa[i] + k)]], "++")] = sa[i];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      sa[i] = nw[i];
      i += 1;
    }
  }
}

func calc(n: dynamic, s: dynamic)
{
  n = n;
  s = s;
  fill(rk, (rk + MAXN), 0);
  {
    var i = 0;
    while ((i < n))
    {
      sa[i] = i;
      rk[i] = (s[i] - ALPHA_OFFSET);
      i += 1;
    }
  }
  {
    var k = 1;
    while ((k < n))
    {
      bucket_sort(k);
      bucket_sort(0);
      nw[sa[0]] = 0;
      {
        var i = 1;
        while ((i < n))
        {
          nw[sa[i]] = (nw[sa[(i - 1)]] + (((rk[sa[i]] != rk[sa[(i - 1)]]) || (rk[(sa[i] + k)] != rk[(sa[(i - 1)] + k)]))));
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < n))
        {
          rk[i] = nw[i];
          i += 1;
        }
      }
      k <<= 1;
    }
  }
  var h = 0;
  lcp[0] = 0;
  lcp[n] = -1;
  {
    var i = 0;
    while ((i < n))
    {
      var j = sa[(rk[i] - 1)];
      {
        ((h > 0) && cpp_update(h, "--"));
        while (((((i + h) < n) && ((j + h) < n)) && (s[(i + h)] == s[(j + h)])))
        {
          h += 1;
        }
      }
      lcp[(rk[i] - 1)] = h;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      st[i][0] = lcp[i];
      i += 1;
    }
  }
  {
    var j = 1;
    while ((j < LOGN))
    {
      {
        var i = 0;
        while ((i <= (n - ((1 << j)))))
        {
          st[i][j] = min(st[i][(j - 1)], st[(i + ((1 << ((j - 1)))))][(j - 1)]);
          i += 1;
        }
      }
      j += 1;
    }
  }
}

func get_lcp(p: dynamic, q: dynamic)
{
  if ((p > q))
  {
    swap(p, q);
  }
  q -= 1;
  var sz = (((8 * cpp_sizeof(dynamic)) - builtin_clz((q - p))) - 1);
  return min(st[p][sz], st[((q - ((1 << sz))) + 1)][sz]);
}

var s = cpp_array(MAXN);

var t = cpp_array(MAXN);

var slen: dynamic;

var tlen: dynamic;

var n: dynamic;

var q: dynamic;

var l = cpp_array(MAXQ);

var r = cpp_array(MAXQ);

var k = cpp_array(MAXQ);

var x = cpp_array(MAXQ);

var y = cpp_array(MAXQ);

var ans = cpp_array(MAXQ);

func cmp_substring(p: dynamic, q: dynamic, len: dynamic)
{
  if ((sfx.get_lcp(sfx.rk[p], sfx.rk[q]) >= len))
  {
    return 0;
  } else
  {
    return if ((sfx.rk[p] < sfx.rk[q])) -1 else +1;
  }
}

func cmp_options(p: dynamic, q: dynamic)
{
  var rev = ((p > q));
  if (rev)
  {
    swap(p, q);
  }
  var cur: dynamic;
  if (((q - p) >= tlen))
  {
    if (((cpp_assign(cur, "=", cmp_substring(p, (slen + 1), tlen))) != 0))
    {
      return (((cur > 0)) ^ rev);
    }
    if (((cpp_assign(cur, "=", cmp_substring((p + tlen), p, ((q - p) - tlen)))) != 0))
    {
      return (((cur > 0)) ^ rev);
    }
    if (((cpp_assign(cur, "=", cmp_substring((slen + 1), (q - tlen), tlen))) != 0))
    {
      return (((cur > 0)) ^ rev);
    }
  } else
  {
    if (((cpp_assign(cur, "=", cmp_substring((slen + 1), p, (q - p)))) != 0))
    {
      return (((cur < 0)) ^ rev);
    }
    if (((cpp_assign(cur, "=", cmp_substring((((slen + 1) + q) - p), (slen + 1), ((tlen - q) + p)))) != 0))
    {
      return (((cur < 0)) ^ rev);
    }
    if (((cpp_assign(cur, "=", cmp_substring(p, ((((slen + 1) + tlen) - q) + p), (q - p)))) != 0))
    {
      return (((cur < 0)) ^ rev);
    }
  }
  return false;
}

var opt_at = cpp_array(MAXN);

var opt_rk = cpp_array(MAXN);

class rmq
{
  var f: dynamic = cpp_array((MAXN / 2), LOGN);
  func build(n: dynamic, arr: dynamic)
  {
      {
        var i = 0;
        while ((i < n))
        {
          f[0][i] = make_pair(arr[i], i);
          i += 1;
        }
      }
      {
        var j = 1;
        while ((j < LOGN))
        {
          {
            var i = 0;
            while ((i <= (n - ((1 << j)))))
            {
              f[j][i] = min(f[(j - 1)][i], f[(j - 1)][(i + ((1 << ((j - 1)))))]);
              i += 1;
            }
          }
          j += 1;
        }
      }
    }
  func query(l: dynamic, r: dynamic)
  {
      if ((l > r))
      {
        return make_pair(MAXN, -1);
      } else if ((l == r))
      {
        return f[0][l];
      }
      var sz = (((8 * cpp_sizeof(dynamic)) - builtin_clz((r - l))) - 1);
      return min(f[sz][l], f[sz][((r - ((1 << sz))) + 1)]);
    }
}

var patrick: dynamic;

func solve_queries()
{
  {
    var i = 0;
    while ((i < q))
    {
      ans[i] = -1;
      i += 1;
    }
  }
  var t = cpp_array(MAXN);
  var idx = cpp_array(MAXN);
  var modulo_seg_start = cpp_array(SOLANUM_TUBEROSUM);
  {
    var cur_k = (SOLANUM_TUBEROSUM - 1);
    while ((cur_k >= 1))
    {
      var ttop = 0;
      {
        var j = 0;
        while ((j < cur_k))
        {
          modulo_seg_start[j] = ttop;
          {
            var k = j;
            while ((k <= slen))
            {
              idx[ttop] = k;
              t[cpp_update(ttop, "++")] = opt_rk[k];
              k += cur_k;
            }
          }
          j += 1;
        }
      }
      patrick.build(ttop, t);
      {
        var i = 0;
        while ((i < q))
        {
          if ((k[i] == cur_k))
          {
            var cur = make_pair(MAXN, -1);
            {
              var rem = x[i];
              while ((rem <= y[i]))
              {
                cur = min(cur, patrick.query(((modulo_seg_start[rem] + cpp_cast(floor((cpp_cast((((l[i] - rem) - 1))) / cur_k)))) + 1), (modulo_seg_start[rem] + cpp_cast(floor((cpp_cast(((r[i] - rem))) / cur_k))))));
                rem += 1;
              }
            }
            ans[i] = if ((cur.second == -1)) -1 else idx[cur.second];
          }
          i += 1;
        }
      }
      cur_k -= 1;
    }
  }
  {
    var i = 0;
    while ((i < q))
    {
      if ((k[i] >= SOLANUM_TUBEROSUM))
      {
        var cur = make_pair(MAXN, -1);
        {
          var mul = 0;
          while ((mul <= slen))
          {
            cur = min(cur, patrick.query(max(l[i], (mul + x[i])), min(r[i], (mul + y[i]))));
            mul += k[i];
          }
        }
        ans[i] = if ((cur.second == -1)) -1 else idx[cur.second];
      }
      i += 1;
    }
  }
}

func main()
{
  {
    slen = 0;
    while (((cpp_assign(s[slen], "=", getchar())) != cpp_char(" ")))
    {
      slen = slen;
      slen += 1;
    }
  }
  s[slen] = cpp_char("\u{0}");
  {
    tlen = 0;
    while (((cpp_assign(t[tlen], "=", getchar())) != cpp_char(" ")))
    {
      tlen = tlen;
      tlen += 1;
    }
  }
  t[tlen] = cpp_char("\u{0}");
  s[slen] = cpp_char("_");
  {
    var i = 0;
    while ((i < tlen))
    {
      s[((slen + 1) + i)] = t[i];
      i += 1;
    }
  }
  s[((slen + tlen) + 1)] = cpp_char("`");
  s[((slen + tlen) + 2)] = cpp_char("\u{0}");
  n = ((slen + tlen) + 2);
  sfx.calc(n, s);
  {
    var i = 0;
    while ((i <= slen))
    {
      opt_at[i] = i;
      i += 1;
    }
  }
  stable_sort(opt_at, ((opt_at + slen) + 1), cmp_options);
  {
    var i = 0;
    while ((i <= slen))
    {
      opt_rk[opt_at[i]] = i;
      i += 1;
    }
  }
  scanf("%d", (&q));
  {
    var i = 0;
    while ((i < q))
    {
      scanf("%d%d%d%d%d", (&l[i]), (&r[i]), (&k[i]), (&x[i]), (&y[i]));
      i += 1;
    }
  }
  solve_queries();
  {
    var i = 0;
    while ((i < q))
    {
      printf("%d%c", ans[i], if ((i == (q - 1))) cpp_char("\n") else cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
