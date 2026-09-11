// Translated from solution.cpp.

var EPS: dynamic = 1e-9;

var INF: dynamic = 1000000005;

var INFF: dynamic = 1000000000000000005;

var PI: dynamic = acos(-1);

var dirx: dynamic = [-1, 0, 0, 1, -1, -1, 1, 1];

var diry: dynamic = [0, 1, -1, 0, -1, 1, -1, 1];

func SIZE(t: dynamic) -> dynamic
{
  return t.size();
}

func SIZE(t: dynamic) -> dynamic
{
  return N;
}

func to_string(t: dynamic) -> dynamic
{
  return (("'" + string_cpp([t])) + "'");
}

func to_string(t: dynamic) -> dynamic
{
  return  (t) ? "true" : "false";
}

func to_string(t: dynamic, x1: dynamic = 0, x2: dynamic = 1e9) -> dynamic
{
  var ret: dynamic = "";
  {
    var i: dynamic = min(x1, SIZE(t));
    var i: dynamic = min(x2, (SIZE(t) - 1));
    while ((i <= i))
    {
      ret += t[i];
      i += 1;
    }
  }
  return ((cpp_char("\"") + ret) + cpp_char("\""));
}

func to_string(t: dynamic) -> dynamic
{
  return to_string(ret);
}

func to_string(t: dynamic, x1: dynamic = 0, x2: dynamic = 1e9) -> dynamic
{
  var ret: dynamic = "";
  {
    var i: dynamic = min(x1, SIZE(t));
    while ((i <= min(x2, (SIZE(t) - 1))))
    {
      ret += (t[i] + cpp_char("0"));
      i += 1;
    }
  }
  return to_string(ret);
}

func to_string(t: dynamic) -> dynamic
{
  return (((("(" + to_string(t.first)) + ", ") + to_string(t.second)) + ")");
}

func to_string(t: dynamic, x1: dynamic, x2: dynamic, C: dynamic...) -> dynamic
{
  var ret: dynamic = "[";
  x1 = min(x1, SIZE(t));
  var e: dynamic = begin(t);
  advance(e, x1);
  {
    var i: dynamic = x1;
    var i: dynamic = min(x2, (SIZE(t) - 1));
    while ((i <= i))
    {
      ret += (to_string((*e), cpp_expand(C)) + ( ((i != i)) ? ", " : ""));
      e = next(e);
      i += 1;
    }
  }
  return (ret + "]");
}

class print_tuple
{
  func operator_call(t: dynamic) -> dynamic
  {
      var ret: dynamic = [](t);
      ret += ( (Index) ? ", " : "");
      return (ret + to_string(get(t)));
    }
}

class print_tuple_0_Ts
{
  func operator_call(t: dynamic) -> dynamic
  {
      return to_string(get(t));
    }
}

func to_string(t: dynamic) -> dynamic
{
  var Size: dynamic = tuple_size.value;
  return [](t);
}

func dbgr() -> dynamic
{
}

func dbgr(H: dynamic, T: dynamic...) -> dynamic
{
  write(to_string(H), " | ");
  dbgr(cpp_expand(T));
}

func dbgs() -> dynamic
{
}

func dbgs(H: dynamic, T: dynamic...) -> dynamic
{
  write(H, " ");
  dbgs(cpp_expand(T));
}

var MOD: dynamic = 1000000007;

func output_vector(v: dynamic, line_break: dynamic = false, add_one: dynamic = false, start: dynamic = -1, end: dynamic = -1) -> dynamic
{
  if ((start < 0))
  {
    start = 0;
  }
  if ((end < 0))
  {
    end = int_cpp(v.size());
  }
  {
    var i: dynamic = start;
    while ((i < end))
    {
      write((v[i] + ( (add_one) ? 1 : 0)), ( (line_break) ? cpp_char("\n") :  ((i < (end - 1))) ? cpp_char(" ") : cpp_char("\n")));
      i += 1;
    }
  }
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    int_cpp(i) = (0);
    while (((i) < (n)))
    {
      read(s[i]);
      (i) += 1;
    }
  }
  {
    int_cpp(i) = (0);
    while (((i) < (n)))
    {
      var t: dynamic = INF;
      {
        int_cpp(k) = (0);
        while (((k) < (2)))
        {
          {
            int_cpp(j) = (0);
            while (((j) < (m)))
            {
              if ((s[i][j] == cpp_char("1")))
              {
                t = 0;
              } else
              {
                t += 1;
              }
              a[i][j] = min(a[i][j], t);
              (j) += 1;
            }
          }
          (k) += 1;
        }
      }
      {
        int_cpp(k) = (0);
        while (((k) < (2)))
        {
          {
            int_cpp(j) = ((m - 1));
            while (((j) >= (0)))
            {
              if ((s[i][j] == cpp_char("1")))
              {
                t = 0;
              } else
              {
                t += 1;
              }
              a[i][j] = min(a[i][j], t);
              (j) -= 1;
            }
          }
          (k) += 1;
        }
      }
      (i) += 1;
    }
  }
  var ans: dynamic = INF;
  {
    int_cpp(i) = (0);
    while (((i) < (m)))
    {
      var t: dynamic = 0;
      {
        int_cpp(j) = (0);
        while (((j) < (n)))
        {
          t = min((t + a[j][i]), INF);
          (j) += 1;
        }
      }
      ans = min(ans, t);
      (i) += 1;
    }
  }
  if ((ans == INF))
  {
    write(-1, cpp_char("\n"));
  } else
  {
    write(ans, cpp_char("\n"));
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  solve();
  return 0;
}
