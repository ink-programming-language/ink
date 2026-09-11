// Translated from solution.cpp.

func nn(c: dynamic) -> dynamic
{
  return (c - cpp_char("a"));
}

func cc(n: dynamic) -> dynamic
{
  return cpp_cast(((n + cpp_char("a"))));
}

class info
{
  var begin: dynamic = cpp_uninitialized();
  var end: dynamic = cpp_uninitialized();
  var transitions: dynamic = cpp_uninitialized();
  func info() -> dynamic
  {
      self->transitions = cpp_construct(100, 0);
    }
  func info(a: dynamic, b: dynamic, c: dynamic) -> dynamic
  {
      self->begin = cpp_construct(a);
      self->end = cpp_construct(b);
      self->transitions = cpp_construct(c);
    }
  func print() -> dynamic
  {
      {
        var i: dynamic = 0;
        while ((i < 10))
        {
          {
            var j: dynamic = 0;
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

var zinfo: dynamic = cpp_construct(cpp_char("z"), cpp_char("z"), vector(100, 0));

func merge(a: dynamic, b: dynamic) -> dynamic
{
  var out: dynamic = cpp_uninitialized();
  out.begin = a.begin;
  out.end = b.end;
  {
    var i: dynamic = 0;
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
  var t: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  func segtree(s: dynamic) -> dynamic
  {
      var sz: dynamic = s.size();
      h = ((cpp_sizeof(dynamic) * 8) - builtin_clz(sz));
      n = (1 << h);
      t = vector((n << 1), zinfo);
      d = vector((n << 1), 0);
      {
        var i: dynamic = 0;
        while ((i < sz))
        {
          t[(i + n)].begin = s[i];
          t[(i + n)].end = s[i];
          i += 1;
        }
      }
      {
        var i: dynamic = (n - 1);
        while ((i > 0))
        {
          pull(i);
          i -= 1;
        }
      }
    }
  func apply(x: dynamic, c: dynamic) -> dynamic
  {
      var g: dynamic = ((cpp_sizeof(dynamic) * 8) - builtin_clz(x));
      var amt: dynamic = (1 << (((h - g) + 1)));
      t[x] = info();
      t[x].begin = c;
      t[x].end = c;
      t[x].transitions[(nn(c) * 11)] = (amt - 1);
      d[x] = c;
    }
  func push(x: dynamic) -> dynamic
  {
      if (((x < n) && (d[x] != 0)))
      {
        apply((x << 1), d[x]);
        apply(((x << 1) | 1), d[x]);
        d[x] = 0;
      }
    }
  func push_to(x: dynamic) -> dynamic
  {
      {
        var i: dynamic = 0;
        while ((i <= h))
        {
          push((x >> ((h - i))));
          i += 1;
        }
      }
    }
  func pull(x: dynamic) -> dynamic
  {
      if ((x == 0))
      {
        return;
      }
      assert((x < n));
      assert((d[x] == 0));
      t[x] = merge(t[(x << 1)], t[((x << 1) | 1)]);
    }
  func pull_from(x: dynamic) -> dynamic
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
  func insert(l: dynamic, r: dynamic, c: dynamic) -> dynamic
  {
      l += n;
      r += n;
      var l0: dynamic = l;
      var r0: dynamic = r;
      push_to(l);
      push_to((r - 1));
      var lst: dynamic = 0;
      var lf: dynamic = 0;
      var rf: dynamic = 0;
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
  func query(s: dynamic) -> dynamic
  {
      var amt: dynamic = 1;
      var pos: dynamic = cpp_construct(10, -1);
      {
        var i: dynamic = 0;
        while ((i < s.size()))
        {
          pos[nn(s[i])] = i;
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < s.size()))
        {
          {
            var j: dynamic = 0;
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, m, k);
  read(s);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var type_cpp: dynamic = cpp_uninitialized();
      read(type_cpp);
      if ((type_cpp == 1))
      {
        var l: dynamic = cpp_uninitialized();
        var r: dynamic = cpp_uninitialized();
        var s: dynamic = cpp_uninitialized();
        read(l, r);
        read(s);
        var c: dynamic = s[0];
        st.insert((l - 1), r, c);
      } else
      {
        var s: dynamic = cpp_uninitialized();
        read(s);
        write(st.query(s), "\n");
      }
      i += 1;
    }
  }
  return 0;
}
