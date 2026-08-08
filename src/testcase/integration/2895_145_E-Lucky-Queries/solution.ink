// Translated from solution.cpp.

class state
{
  var total: dynamic;
  var ss: dynamic;
  var se: dynamic;
  var ee: dynamic;
  var size: dynamic;
  func state()
  {
      this->total = cpp_construct(0);
      this->ss = cpp_construct(0);
      this->se = cpp_construct(0);
      this->ee = cpp_construct(0);
      this->size = cpp_construct(0);
    }
  func build_base(c: dynamic, startchar: dynamic)
  {
      var s: dynamic;
      s.total = 1;
      s.size = 1;
      if ((c == startchar))
      {
        s.ss = 1;
      } else
      {
        s.ee = 1;
      }
      return s;
    }
  func build_inc(c: dynamic)
  {
      return build_base(c, cpp_char("4"));
    }
  func build_dec(c: dynamic)
  {
      return build_base(c, cpp_char("7"));
    }
}

func merge(l: dynamic, r: dynamic)
{
  var s: dynamic;
  s.size = (l.size + r.size);
  s.total = max(l.total, r.total);
  s.total = max(s.total, (l.ss + max(max(r.ss, r.se), r.ee)));
  s.total = max(s.total, (max(l.se, l.ee) + r.ee));
  s.ss = (l.ss + r.ss);
  s.se = max(max((l.ss + r.se), (l.ss + r.ee)), (l.se + r.ee));
  s.ee = (l.ee + r.ee);
  return s;
}

func operator_shift_left(os: dynamic, s: dynamic)
{
  (os << "(");
  (((((((((os << s.total) << ",") << s.ss) << ",") << s.se) << ",") << s.ee) << ",") << s.size);
  (os << ")");
  return os;
}

class segtree
{
  var t_inc: dynamic;
  var t_dec: dynamic;
  var d: dynamic;
  var h: dynamic;
  var n: dynamic;
  func segtree(s: dynamic)
  {
      var sz = s.size();
      h = ((cpp_sizeof(dynamic) * 8) - builtin_clz(sz));
      n = (1 << h);
      d = vector((n << 1), 0);
      t_inc = vector((n << 1));
      t_dec = vector((n << 1));
      {
        var i = 0;
        while ((i < s.size()))
        {
          t_inc[(i + n)] = state.build_inc(s[i]);
          t_dec[(i + n)] = state.build_dec(s[i]);
          i += 1;
        }
      }
      {
        var i = (n - 1);
        while ((i > 0))
        {
          t_inc[i] = merge(t_inc[(i << 1)], t_inc[((i << 1) | 1)]);
          t_dec[i] = merge(t_dec[(i << 1)], t_dec[((i << 1) | 1)]);
          i -= 1;
        }
      }
    }
  func build(x: dynamic)
  {
      {
        x /= 2;
        while ((x > 0))
        {
          t_inc[x] = merge(t_inc[(x << 1)], t_inc[((x << 1) | 1)]);
          t_dec[x] = merge(t_dec[(x << 1)], t_dec[((x << 1) | 1)]);
          x /= 2;
        }
      }
    }
  func apply(x: dynamic)
  {
      d[x] += 1;
      swap(t_inc[x], t_dec[x]);
    }
  func push(x: dynamic)
  {
      {
        var s = h;
        while ((s > 0))
        {
          var i = (x >> s);
          if ((d[i] % 2))
          {
            d[i] = 0;
            apply((i << 1));
            apply(((i << 1) | 1));
          }
          s -= 1;
        }
      }
    }
  func flip(l: dynamic, r: dynamic)
  {
      var l0 = (l + n);
      var r0 = (r + n);
      {
        l += n;
        r += n;
        while ((l < r))
        {
          if ((l % 2))
          {
            apply(cpp_update(l, "++"));
          }
          if ((r % 2))
          {
            apply(cpp_update(r, "--"));
          }
          l /= 2;
          r /= 2;
        }
      }
      push(l0);
      push((r0 - 1));
      build(l0);
      build((r0 - 1));
    }
  func query()
  {
      return t_inc[1].total;
    }
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var N: dynamic;
  var Q: dynamic;
  read(N, Q);
  var s: dynamic;
  read(s);
  {
    var qq = 0;
    while ((qq < Q))
    {
      var op: dynamic;
      read(op);
      if ((op[0] == cpp_char("c")))
      {
        write(st.query(), "\n");
      } else
      {
        var l: dynamic;
        var r: dynamic;
        read(l, r);
        st.flip((l - 1), r);
      }
      qq += 1;
    }
  }
  return 0;
}
