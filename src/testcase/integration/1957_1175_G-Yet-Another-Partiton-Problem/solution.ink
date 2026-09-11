// Translated from solution.cpp.

class dreapta
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  func operator_call(x: dynamic) -> dynamic
  {
      return ((a * x) + b);
    }
}

func useless(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  assert(((a.a >= b.a) && (b.a >= c.a)));
  return ((cpp_cast(((b.b - a.b))) / ((a.a - b.a))) > (cpp_cast(((c.b - b.b))) / ((b.a - c.a))));
}

class merging_batch
{
  var buf: dynamic = cpp_uninitialized();
  func merging_batch() -> dynamic
  {
    }
  func query(x: dynamic) -> dynamic
  {
      var i: dynamic = 0;
      {
        var step: dynamic = (1 << 23);
        while (step)
        {
          if ((((i + step) < buf.size()) && (buf[(i + step)](x) <= buf[((i + step) - 1)](x))))
          {
            i += step;
          }
          step /= 2;
        }
      }
      assert(((i == (buf.size() - 1)) || (buf[i](x) <= buf[(i + 1)](x))));
      return buf[i](x);
    }
  func add_right(x: dynamic) -> dynamic
  {
      while (((buf.size() > 1) && useless(buf.rbegin()[1], buf.back(), x)))
      {
        buf.pop_back();
      }
      buf.push_back(x);
    }
  func add_left(x: dynamic) -> dynamic
  {
      while (((buf.size() > 1) && useless(x, buf[0], buf[1])))
      {
        buf.pop_front();
      }
      buf.push_front(x);
    }
  func swap_with(rhs: dynamic) -> dynamic
  {
      swap(buf, rhs.buf);
    }
  func merge_with(rhs: dynamic) -> dynamic
  {
      if ((buf.size() < rhs.buf.size()))
      {
        {
          var it: dynamic = buf.rbegin();
          while ((it != buf.rend()))
          {
            rhs.add_left((*it));
            it += 1;
          }
        }
        swap(buf, rhs.buf);
      } else
      {
        for (var x: dynamic in rhs.buf)
        {
          add_right(x);
        }
      }
    }
}

class undo_batch
{
  var top: dynamic = cpp_uninitialized();
  var st: dynamic = cpp_uninitialized();
  var undo_st: dynamic = cpp_uninitialized();
  func undo_batch() -> dynamic
  {
      top = 1;
    }
  func clear() -> dynamic
  {
      top = 1;
    }
  func print() -> dynamic
  {
      {
        var i: dynamic = 1;
        while ((i < top))
        {
          write("(", st[i].a, ",", st[i].b, ") ");
          i += 1;
        }
      }
      write("\n");
    }
  func query(x: dynamic) -> dynamic
  {
      var i: dynamic = 0;
      {
        var step: dynamic = (1 << 23);
        while (step)
        {
          if ((((i + step) < top) && (st[(i + step)](x) <= st[((i + step) - 1)](x))))
          {
            i += step;
          }
          step /= 2;
        }
      }
      assert(((i == (top - 1)) || (st[i](x) <= st[(i + 1)](x))));
      return st[i](x);
    }
  func undo() -> dynamic
  {
      assert((!undo_st.empty()));
      top = undo_st.back().old_top;
      st[undo_st.back().old_pos] = undo_st.back().d;
      undo_st.pop_back();
    }
  func add_right(d: dynamic) -> dynamic
  {
      undo_st.push_back([top, 0, 0]);
      var ret: dynamic = 0;
      {
        var step: dynamic = (1 << 23);
        while (step)
        {
          if ((((ret + step) < top) && ((!useless(st[((ret + step) - 1)], st[(ret + step)], d)))))
          {
            ret += step;
          }
          step /= 2;
        }
      }
      top = (ret + 2);
      undo_st.back().old_pos = (ret + 1);
      undo_st.back().d = st[(ret + 1)];
      st[(ret + 1)] = d;
    }
}

func produce_state(n: dynamic, d: dynamic, v: dynamic, ret: dynamic) -> dynamic
{
  var posz: dynamic = cpp_uninitialized();
  var mb: dynamic = cpp_uninitialized();
  var ub: dynamic = cpp_uninitialized();
  posz.clear();
  mb.clear();
  ub.clear();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      x.add_right([((-i) + 1), d[i]]);
      while (((!posz.empty()) && (v[posz.back()] <= v[i])))
      {
        mb.back().merge_with(x);
        x.swap_with(mb.back());
        mb.pop_back();
        posz.pop_back();
        ub.undo();
      }
      posz.push_back(i);
      mb.push_back(move(x));
      ub.add_right([v[i], mb.back().query(v[i])]);
      ret[(i + 1)] = ub.query(i);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  for (var x: dynamic in v)
  {
    read(x);
  }
  var state: dynamic = cpp_construct((n + 1), (cpp_cast(1e9) * cpp_cast(1e9)));
  var tmp: dynamic = cpp_construct((n + 1), 0);
  state[0] = 0;
  tmp[0] = 0;
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      produce_state(n, state, v, tmp);
      swap(state, tmp);
      i += 1;
    }
  }
  write(state.back(), "\n");
  return 0;
}
