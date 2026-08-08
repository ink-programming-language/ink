// Translated from solution.cpp.

func REP(i: dynamic, st: dynamic, ed: dynamic)
{
  cpp_macro("for(register int i=st,i##end=ed;i<=i##end;++i)");
}

func DREP(i: dynamic, st: dynamic, ed: dynamic)
{
  cpp_macro("for(register int i=st,i##end=ed;i>=i##end;--i)");
}

func chkmin(x: dynamic, y: dynamic)
{
  return if (((y < x))) (cpp_comma(x, cpp_expression("=y"), 1)) else 0;
}

func chkmax(x: dynamic, y: dynamic)
{
  return if (((y > x))) (cpp_comma(x, cpp_expression("=y"), 1)) else 0;
}

func read()
{
  var x: dynamic;
  var c: dynamic;
  var f = 1;
  while ((((cpp_assign(c, "=", getchar())) != cpp_char("-")) && (((c > cpp_char("9")) || (c < cpp_char("0"))))))
  {
  }
  if ((c == cpp_char("-")))
  {
    f = -1;
    c = getchar();
  }
  x = (c ^ cpp_char("0"));
  while ((((cpp_assign(c, "=", getchar())) >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = ((((x << 1)) + ((x << 3))) + ((c ^ cpp_char("0"))));
  }
  return (x * f);
}

func readll()
{
  var x: dynamic;
  var c: dynamic;
  var f = 1;
  while ((((cpp_assign(c, "=", getchar())) != cpp_char("-")) && (((c > cpp_char("9")) || (c < cpp_char("0"))))))
  {
  }
  if ((c == cpp_char("-")))
  {
    f = -1;
    c = getchar();
  }
  x = (c ^ cpp_char("0"));
  while ((((cpp_assign(c, "=", getchar())) >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = ((((x << 1)) + ((x << 3))) + ((c ^ cpp_char("0"))));
  }
  return (x * f);
}

var maxn = (2e5 + 10);

var inf = 0x3f3f3f3f;

var idx = cpp_array(maxn);

var idx_cnt: dynamic;

var a = cpp_array(maxn);

class Segment_tree
{
  var Min: dynamic = cpp_array((maxn << 2));
  var tag: dynamic = cpp_array((maxn << 2));
  func push_down(x: dynamic)
  {
      if (tag[x])
      {
        Min[(x << 1)] += tag[x];
        Min[((x << 1) | 1)] += tag[x];
        tag[(x << 1)] += tag[x];
        tag[((x << 1) | 1)] += tag[x];
        tag[x] = 0;
      }
    }
  func push_up(x: dynamic)
  {
      Min[x] = min(Min[(x << 1)], Min[((x << 1) | 1)]);
    }
  func build_tree(x: dynamic, L: dynamic, R: dynamic)
  {
      if ((L == R))
      {
        Min[x] = idx[L];
        return;
      }
      var Mid = (((L + R)) >> 1);
      build_tree((x << 1), L, Mid);
      build_tree(((x << 1) | 1), (Mid + 1), R);
      push_up(x);
    }
  func update(x: dynamic, L: dynamic, R: dynamic, ql: dynamic, qr: dynamic, v: dynamic)
  {
      if ((ql > qr))
      {
        return;
      }
      if (((ql <= L) && (R <= qr)))
      {
        tag[x] += v;
        Min[x] += v;
        return;
      }
      var Mid = (((L + R)) >> 1);
      push_down(x);
      if ((ql <= Mid))
      {
        update((x << 1), L, Mid, ql, qr, v);
      }
      if ((qr > Mid))
      {
        update(((x << 1) | 1), (Mid + 1), R, ql, qr, v);
      }
      push_up(x);
    }
  func query(x: dynamic, L: dynamic, R: dynamic, ql: dynamic, qr: dynamic)
  {
      if (((ql <= L) && (R <= qr)))
      {
        return Min[x];
      }
      var Mid = (((L + R)) >> 1);
      var res = inf;
      push_down(x);
      if ((ql <= Mid))
      {
        chkmin(res, query((x << 1), L, Mid, ql, qr));
      }
      if ((qr > Mid))
      {
        chkmin(res, query(((x << 1) | 1), (Mid + 1), R, ql, qr));
      }
      push_up(x);
      return res;
    }
}

var Seg: dynamic;

var ans: dynamic;

func main()
{
  var n = read();
  REP(i, 1, n)[i] = read();
  REP(i, 1, n)[cpp_update(idx_cnt, "++")] = a[i];
  sort((idx + 1), ((idx + idx_cnt) + 1));
  idx_cnt = ((unique((idx + 1), ((idx + idx_cnt) + 1)) - idx) - 1);
  Seg.build_tree(1, 1, idx_cnt);
  REP(i, 1, n);
  {
    var u = (lower_bound((idx + 1), ((idx + idx_cnt) + 1), a[i]) - idx);
    ans += (Seg.query(1, 1, idx_cnt, u, idx_cnt) - a[i]);
    Seg.update(1, 1, idx_cnt, 1, (u - 1), 1);
  }
  printf("%lld\n", ans);
  return 0;
}
