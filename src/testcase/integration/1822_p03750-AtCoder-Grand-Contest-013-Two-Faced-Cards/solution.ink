// Translated from solution.cpp.

func cpp_ref(i: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  cpp_macro("for(int i=x;i<=y;++i)");
}

func def(i: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  cpp_macro("for(int i=x;i>=y;--i)");
}

var pb: dynamic = cpp_expression("#include");

func SZ(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/");
}

var mp: dynamic = cpp_expression("#include");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

var N: dynamic = 500010;

func read() -> dynamic
{
  var c: dynamic = getchar();
  var d: dynamic = 0;
  var f: dynamic = 1;
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

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(N);

var res: dynamic = cpp_array(N);

var a: dynamic = cpp_array(N);

var S: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var px: dynamic = cpp_uninitialized();

var py: dynamic = cpp_uninitialized();

var pz: dynamic = cpp_uninitialized();

var rd: dynamic = cpp_uninitialized();

var Rd: dynamic = cpp_array(N);

func cmpse(a: dynamic, b: dynamic) -> dynamic
{
  return (a.se < b.se);
}

func upd(x: dynamic, s: dynamic) -> dynamic
{
  {
    while ((x <= m))
    {
      a[x] += s;
      x += (x & (-x));
    }
  }
}

func ask(x: dynamic) -> dynamic
{
  var s: dynamic = 0;
  {
    while (x)
    {
      s += a[x];
      x -= (x & (-x));
    }
  }
  return s;
}

func main() -> dynamic
{
  n = read();
  cpp_ref(i, 1, ((n * 3) + 1)).pb(mp(read(), i));
  q = read();
  cpp_ref(i, 1, (q * 2)).pb(mp(read(), (((n * 3) + 1) + i)));
  sort(rd.begin(), rd.end());
  {
    var i: dynamic = 0;
    var la: dynamic = -1;
    while ((i < SZ(rd)))
    {
      m += (rd[i].fi != la);
      la = rd[i].fi;
      Rd[rd[i].se] = m;
      i += 1;
    }
  }
  cpp_ref(i, 1, n);
  {
    var a: dynamic = Rd[cpp_update(cnt, "++")];
    var b: dynamic = Rd[cpp_update(cnt, "++")];
    s[a] += 1;
    if ((b < a))
    {
      p.pb(mp(b, (a - 1)));
    }
  }
  cpp_ref(i, 1, (n + 1))[Rd[cpp_update(cnt, "++")]] -= 1;
  cpp_ref(i, 1, m);
  if (s[i])
  {
    upd(i, s[i]);
  }
  px = cpp_assign(py, "=", p);
  cpp_ref(i, 0, (SZ(px) - 1));
  swap(px[i].fi, px[i].se);
  sort(px.begin(), px.end(), cmpse);
  sort(py.begin(), py.end(), cmpse);
  S.clear();
  S.insert(mp(1e9, 1e9));
  var flag: dynamic = 1;
  var rs: dynamic = n;
  {
    var i: dynamic = m;
    var cntb: dynamic = (SZ(py) - 1);
    while ((i >= 1))
    {
      var s: dynamic = ask(i);
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
        var it: dynamic = S.lower_bound(mp(0, 0));
        var w: dynamic = (*it);
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
  cpp_ref(i, 1, m)[i] = -1e9;
  {
    var i: dynamic = 1;
    var cnta: dynamic = 0;
    var cntb: dynamic = 0;
    while ((i <= m))
    {
      res[i] = rs;
      var s: dynamic = ask(i);
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
        var it: dynamic = cpp_update(S.lower_bound(mp(1e9, 1e9)), "--");
        var w: dynamic = (*it);
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
  cpp_ref(i, 1, q);
  {
    var a: dynamic = Rd[cpp_update(cnt, "++")];
    var b: dynamic = Rd[cpp_update(cnt, "++")];
    var ans: dynamic = max((res[a] + 1), res[b]);
    if ((ans < 0))
    {
      ans = -1;
    }
    printf("%d\n", ans);
  }
}
