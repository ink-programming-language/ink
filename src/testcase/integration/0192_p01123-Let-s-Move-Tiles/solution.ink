// Translated from solution.cpp.

func ALL(v: dynamic) -> dynamic
{
  return cpp_expression("#include <iostr");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);++i)");
}

func RREP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=(n)-1;i>=0;--i)");
}

var targets: dynamic = [[1, 1, 2, 2, 2, 1, 8, 8, 8], [3, 2, 2, 3, 4, 4, 4, 3, 2], [5, 5, 4, 4, 4, 5, 6, 6, 6], [7, 8, 8, 7, 6, 6, 6, 7, 8]];

func getdir(c: dynamic) -> dynamic
{
  var __cpp_switch_1: dynamic = c;
  if (__cpp_switch_1 == cpp_char("R"))
  {
    return 0;
  }
  else if (__cpp_switch_1 == cpp_char("U"))
  {
    return 1;
  }
  else if (__cpp_switch_1 == cpp_char("L"))
  {
    return 2;
  }
  else if (__cpp_switch_1 == cpp_char("D"))
  {
    return 3;
  }
  abort();
}

class solver
{
  var n: dynamic = cpp_uninitialized();
  func solver(n: dynamic) -> dynamic
  {
      self->n = cpp_construct(n);
    }
  var ids: dynamic = cpp_array(9);
  var trs: dynamic = cpp_array(4);
  var unit: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  func mul(x: dynamic, y: dynamic) -> dynamic
  {
      var sz: dynamic = x.size();
      return ret;
    }
  func pow(x: dynamic, y: dynamic) -> dynamic
  {
      var a: dynamic = unit;
      while (y)
      {
        if ((y & 1))
        {
          a = mul(a, x);
        }
        x = mul(x, x);
        y >>= 1;
      }
      return a;
    }
  func parse() -> dynamic
  {
      var a: dynamic = unit;
      var b: dynamic = cpp_uninitialized();
      while (1)
      {
        if (((*p) == cpp_char("(")))
        {
          p += 1;
          var x: dynamic = parse();
          p += 1;
          var endp: dynamic = cpp_uninitialized();
          var r: dynamic = strtoll(p, (&endp), 10);
          p = endp;
          b = pow(x, r);
        } else if (isalpha((*p)))
        {
          var dir: dynamic = getdir((*p));
          p += 1;
          b = trs[dir];
        } else
        {
          break;
        }
        a = mul(a, b);
      }
      return a;
    }
  func solve() -> dynamic
  {
      var op: dynamic = cpp_uninitialized();
      read(op);
      var fstdir: dynamic = -1;
      for (var c: dynamic in op)
      {
        if (isalpha(c))
        {
          fstdir = getdir(c);
          break;
        }
      }
      var in_cpp: dynamic = cpp_construct(n, string_cpp(n, cpp_char(".")));
      if ((fstdir == 0))
      {
      } else if ((fstdir == 1))
      {
      } else if ((fstdir == 2))
      {
      } else
      {
      }
      REP(i, 9);
      {
        ids[i].assign(n, vint(n, -1));
      }
      var id: dynamic = 0;
      REP(i, n);
      var pcnt: dynamic = id;
      REP(i, 4);
      {
        trs[i].assign((9 * pcnt), -1);
      }
      unit.resize((9 * pcnt));
      iota(ALL(unit), 0);
      REP(from_cpp, 9);
      {
        var to: dynamic = cpp_uninitialized();
        var dir: dynamic = cpp_uninitialized();
        dir = 0;
        to = targets[dir][from_cpp];
        id = (to * pcnt);
        dir = 1;
        to = targets[dir][from_cpp];
        id = (to * pcnt);
        dir = 2;
        to = targets[dir][from_cpp];
        id = (to * pcnt);
        dir = 3;
        to = targets[dir][from_cpp];
        id = (to * pcnt);
      }
      p = op.c_str();
      var res: dynamic = parse();
      var val: dynamic = cpp_construct((9 * pcnt));
      REP(y, n);
      var ans: dynamic = cpp_construct(n, string_cpp(n, cpp_char(".")));
      REP(i, 9);
      REP(y, n);
    }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while (((cin >> n) && n))
  {
    solver(n).solve();
  }
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      ret[i] = y[x[i]];
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      read(in0[i]);
    }

func RREP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
          if ((in0[y][x] != cpp_char(".")))
          {
            in_cpp[y][cpp_update(u, "--")] = in0[y][x];
          }
        }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var u: dynamic = (n - 1);
      }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
          if ((in0[y][x] != cpp_char(".")))
          {
            in_cpp[cpp_update(u, "++")][x] = in0[y][x];
          }
        }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var u: dynamic = 0;
      }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
          if ((in0[y][x] != cpp_char(".")))
          {
            in_cpp[y][cpp_update(u, "++")] = in0[y][x];
          }
        }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var u: dynamic = 0;
      }

func RREP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
          if ((in0[y][x] != cpp_char(".")))
          {
            in_cpp[cpp_update(u, "--")][x] = in0[y][x];
          }
        }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var u: dynamic = (n - 1);
      }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if ((in_cpp[i][j] != cpp_char(".")))
      {
        ids[0][i][j] = id;
        id += 1;
      }
    }

func RREP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
          if ((ids[from_cpp][y][x] >= 0))
          {
            var r: dynamic = ids[to][y][u];
            u -= 1;
            if ((r == -1))
            {
              r = cpp_update(id, "++");
            }
            trs[dir][ids[from_cpp][y][x]] = r;
          }
        }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var u: dynamic = (n - 1);
      }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
          if ((ids[from_cpp][y][x] >= 0))
          {
            var r: dynamic = ids[to][u][x];
            u += 1;
            if ((r == -1))
            {
              r = cpp_update(id, "++");
            }
            trs[dir][ids[from_cpp][y][x]] = r;
          }
        }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var u: dynamic = 0;
      }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
          if ((ids[from_cpp][y][x] >= 0))
          {
            var r: dynamic = ids[to][y][u];
            u += 1;
            if ((r == -1))
            {
              r = cpp_update(id, "++");
            }
            trs[dir][ids[from_cpp][y][x]] = r;
          }
        }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var u: dynamic = 0;
      }

func RREP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
          if ((ids[from_cpp][y][x] >= 0))
          {
            var r: dynamic = ids[to][u][x];
            u -= 1;
            if ((r == -1))
            {
              r = cpp_update(id, "++");
            }
            trs[dir][ids[from_cpp][y][x]] = r;
          }
        }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var u: dynamic = (n - 1);
      }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var k: dynamic = ids[0][y][x];
      if ((k >= 0))
      {
        val[res[k]] = in_cpp[y][x];
      }
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var k: dynamic = ids[i][y][x];
      if (((k >= 0) && (val[k] != 0)))
      {
        ans[y][x] = val[k];
      }
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      write(ans[i], cpp_char("\n"));
    }
