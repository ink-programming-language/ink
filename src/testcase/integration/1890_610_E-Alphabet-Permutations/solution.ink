// Translated from solution.cpp.

func nn(c: dynamic)
{
  return (c - cpp_char("a"));
}

func cc(n: dynamic)
{
  return cpp_cast(((n + cpp_char("a"))));
}

class info
{
  var begin: dynamic;
  var end: dynamic;
  var transitions: dynamic;
  func info()
  {
      this->transitions = cpp_construct(100, 0);
    }
  func info(a: dynamic, b: dynamic, c: dynamic)
  {
      this->begin = cpp_construct(a);
      this->end = cpp_construct(b);
      this->transitions = cpp_construct(c);
    }
  func print()
  {
      {
        var i = 0;
        while ((i < 10))
        {
          {
            var j = 0;
            while ((j < 10))
            {
              write(cc(i), cc(j), transitions[((i * 10) + j)], " ");
              j += 1;
            }
          }
          write("\n");
          i += 1;
        }
      }
      write("\n");
    }
}

var zinfo = cpp_construct(cpp_char("z"), cpp_char("z"), vector(100, 0));

func merge(a: dynamic, b: dynamic)
{
  var out: dynamic;
  out.begin = a.begin;
  out.end = b.end;
  {
    var i = 0;
    while ((i < 100))
    {
      out.transitions[i] = (a.transitions[i] + b.transitions[i]);
      i += 1;
    }
  }
  if (((a.end < cpp_char("o")) && (b.begin < cpp_char("o"))))
  {
    out.transitions[((nn(a.end) * 10) + nn(b.begin))] += 1;
  }
  return out;
}

class segtree
{
  var t: dynamic;
  var d: dynamic;
  var n: dynamic;
  var h: dynamic;
  func segtree(s: dynamic)
  {
      var sz = s.size();
      h = ((cpp_sizeof(dynamic) * 8) - builtin_clz(sz));
      n = (1 << h);
      t = vector((n << 1), zinfo);
      d = vector((n << 1), 0);
      {
        var i = 0;
        while ((i < sz))
        {
          t[(i + n)].begin = s[i];
          t[(i + n)].end = s[i];
          i += 1;
        }
      }
      {
        var i = (n - 1);
        while ((i > 0))
        {
          pull(i);
          i -= 1;
        }
      }
    }
  func apply(x: dynamic, c: dynamic)
  {
      var g = ((cpp_sizeof(dynamic) * 8) - builtin_clz(x));
      var amt = (1 << (((h - g) + 1)));
      t[x] = info();
      t[x].begin = c;
      t[x].end = c;
      t[x].transitions[(nn(c) * 11)] = (amt - 1);
      d[x] = c;
    }
  func push(x: dynamic)
  {
      if (((x < n) && (d[x] != 0)))
      {
        apply((x << 1), d[x]);
        apply(((x << 1) | 1), d[x]);
        d[x] = 0;
      }
    }
  func push_to(x: dynamic)
  {
      {
        var i = 0;
        while ((i <= h))
        {
          push((x >> ((h - i))));
          i += 1;
        }
      }
    }
  func pull(x: dynamic)
  {
      if ((x == 0))
      {
        return;
      }
      assert((x < n));
      assert((d[x] == 0));
      t[x] = merge(t[(x << 1)], t[((x << 1) | 1)]);
    }
  func pull_from(x: dynamic)
  {
      {
        x /= 2;
        while ((x > 0))
        {
          pull(x);
          x /= 2;
        }
      }
    }
  func insert(l: dynamic, r: dynamic, c: dynamic)
  {
      l += n;
      r += n;
      var l0 = l;
      var r0 = r;
      push_to(l);
      push_to((r - 1));
      var lst = 0;
      var lf = 0;
      var rf = 0;
      {
        while ((l < r))
        {
          lst = l;
          if ((l % 2))
          {
            lf = max(lf, l);
            apply(cpp_update(l, "++"), c);
          }
          if ((r % 2))
          {
            apply(cpp_update(r, "--"), c);
            rf = max(rf, r);
          }
          l /= 2;
          r /= 2;
        }
      }
      pull_from(rf);
      pull_from(lf);
    }
  func query(s: dynamic)
  {
      var amt = 1;
      var pos = cpp_construct(10, -1);
      {
        var i = 0;
        while ((i < s.size()))
        {
          pos[nn(s[i])] = i;
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < s.size()))
        {
          {
            var j = 0;
            while ((j < s.size()))
            {
              if ((pos[i] >= pos[j]))
              {
                amt += t[1].transitions[((i * 10) + j)];
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      return amt;
    }
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  var s: dynamic;
  read(n, m, k);
  read(s);
  {
    var i = 0;
    while ((i < m))
    {
      var type_cpp: dynamic;
      read(type_cpp);
      if ((type_cpp == 1))
      {
        var l: dynamic;
        var r: dynamic;
        var s: dynamic;
        read(l, r);
        read(s);
        var c = s[0];
        st.insert((l - 1), r, c);
      } else
      {
        var s: dynamic;
        read(s);
        write(st.query(s), "\n");
      }
      i += 1;
    }
  }
  return 0;
}
