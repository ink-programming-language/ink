// Translated from solution.cpp.

func ref(i: dynamic, x: dynamic, y: dynamic)
{
  cpp_macro("for(int i=x;i<=y;++i)");
}

func def(i: dynamic, x: dynamic, y: dynamic)
{
  cpp_macro("for(int i=x;i>=y;--i)");
}

var pb = cpp_expression("#include");

func SZ(x: dynamic)
{
  return cpp_expression("#include <bits/");
}

var mp = cpp_expression("#include");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

var N = 500010;

func read()
{
  var c = getchar();
  var d = 0;
  var f = 1;
  {
    while (((c < cpp_char("0")) || (c > cpp_char("9"))))
    {
      if ((c == cpp_char("-")))
      {
        f = -1;
      }
      c = getchar();
    }
  }
  {
    while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
    {
      d = (((d * 10) + c) - 48);
      c = getchar();
    }
  }
  return (d * f);
}

var n: dynamic;

var q: dynamic;

var m: dynamic;

var cnt: dynamic;

var s = cpp_array(N);

var res = cpp_array(N);

var a = cpp_array(N);

var S: dynamic;

var p: dynamic;

var px: dynamic;

var py: dynamic;

var pz: dynamic;

var rd: dynamic;

var Rd = cpp_array(N);

func cmpse(a: dynamic, b: dynamic)
{
  return (a.se < b.se);
}

func upd(x: dynamic, s: dynamic)
{
  {
    while ((x <= m))
    {
      a[x] += s;
      x += (x & (-x));
    }
  }
}

func ask(x: dynamic)
{
  var s = 0;
  {
    while (x)
    {
      s += a[x];
      x -= (x & (-x));
    }
  }
  return s;
}

func main()
{
  n = read();
  ref(i, 1, ((n * 3) + 1)).pb(mp(read(), i));
  q = read();
  ref(i, 1, (q * 2)).pb(mp(read(), (((n * 3) + 1) + i)));
  sort(rd.begin(), rd.end());
  {
    var i = 0;
    var la = -1;
    while ((i < SZ(rd)))
    {
      m += (rd[i].fi != la);
      la = rd[i].fi;
      Rd[rd[i].se] = m;
      i += 1;
    }
  }
  ref(i, 1, n);
  {
    var a = Rd[cpp_update(cnt, "++")];
    var b = Rd[cpp_update(cnt, "++")];
    s[a] += 1;
    if ((b < a))
    {
      p.pb(mp(b, (a - 1)));
    }
  }
  ref(i, 1, (n + 1))[Rd[cpp_update(cnt, "++")]] -= 1;
  ref(i, 1, m);
  if (s[i])
  {
    upd(i, s[i]);
  }
  px = cpp_assign(py, "=", p);
  ref(i, 0, (SZ(px) - 1));
  swap(px[i].fi, px[i].se);
  sort(px.begin(), px.end(), cmpse);
  sort(py.begin(), py.end(), cmpse);
  S.clear();
  S.insert(mp(1e9, 1e9));
  var flag = 1;
  var rs = n;
  {
    var i = m;
    var cntb = (SZ(py) - 1);
    while ((i >= 1))
    {
      var s = ask(i);
      if ((s >= -1))
      {
        i -= 1;
        continue;
      }
      s = (-1 - s);
      while (((cntb >= 0) && (py[cntb].se >= i)))
      {
        S.insert(py[cpp_update(cntb, "--")]);
      }
      while (s)
      {
        var it = S.lower_bound(mp(0, 0));
        var w = (*it);
        if ((w.fi > i))
        {
          flag = 0;
          break;
        }
        upd((w.se + 1), -1);
        upd(w.fi, 1);
        S.erase(it);
        pz.pb(mp(w.se, w.fi));
        rs -= 1;
        s -= 1;
      }
      if ((!flag))
      {
        break;
      }
      i -= 1;
    }
  }
  sort(pz.begin(), pz.end(), cmpse);
  if ((!flag))
  {
    cpp_statement("ref(i,1,q)");
    puts("-1");
    return 0;
  }
  S.clear();
  S.insert(mp(1e9, 1e9));
  S.insert(mp(0, 0));
  ref(i, 1, m)[i] = -1e9;
  {
    var i = 1;
    var cnta = 0;
    var cntb = 0;
    while ((i <= m))
    {
      res[i] = rs;
      var s = ask(i);
      if ((s >= 0))
      {
        i += 1;
        continue;
      }
      s = (-s);
      while (((cnta < SZ(px)) && (px[cnta].se <= i)))
      {
        S.insert(px[cpp_update(cnta, "++")]);
      }
      while (((cntb < SZ(pz)) && (pz[cntb].se <= i)))
      {
        S.erase(S.lower_bound(pz[cpp_update(cntb, "++")]));
      }
      while (s)
      {
        var it = cpp_update(S.lower_bound(mp(1e9, 1e9)), "--");
        var w = (*it);
        if ((w.fi < i))
        {
          flag = 0;
          break;
        }
        upd((w.fi + 1), -1);
        upd(w.se, 1);
        S.erase(it);
        rs -= 1;
        s -= 1;
      }
      if ((!flag))
      {
        break;
      }
      i += 1;
    }
  }
  ref(i, 1, q);
  {
    var a = Rd[cpp_update(cnt, "++")];
    var b = Rd[cpp_update(cnt, "++")];
    var ans = max((res[a] + 1), res[b]);
    if ((ans < 0))
    {
      ans = -1;
    }
    printf("%d\n", ans);
  }
}
