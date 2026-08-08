// Translated from solution.cpp.

var EPS = 1e-9;

var INF = 1000000005;

var INFF = 1000000000000000005;

var PI = acos(-1);

var dirx = [-1, 0, 0, 1, -1, -1, 1, 1];

var diry = [0, 1, -1, 0, -1, 1, -1, 1];

func SIZE(t: dynamic)
{
  return t.size();
}

func SIZE(t: dynamic)
{
  return N;
}

func to_string(t: dynamic)
{
  return (("'" + string_cpp([t])) + "'");
}

func to_string(t: dynamic)
{
  return if (t) "true" else "false";
}

func to_string(t: dynamic, x1: dynamic = 0, x2: dynamic = 1e9)
{
  var ret = "";
  {
    var i = min(x1, SIZE(t));
    var i = min(x2, (SIZE(t) - 1));
    while ((i <= i))
    {
      ret += t[i];
      i += 1;
    }
  }
  return ((cpp_char("\"") + ret) + cpp_char("\""));
}

func to_string(t: dynamic)
{
  return to_string(ret);
}

func to_string(t: dynamic, x1: dynamic = 0, x2: dynamic = 1e9)
{
  var ret = "";
  {
    var i = min(x1, SIZE(t));
    while ((i <= min(x2, (SIZE(t) - 1))))
    {
      ret += (t[i] + cpp_char("0"));
      i += 1;
    }
  }
  return to_string(ret);
}

func to_string(t: dynamic)
{
  return (((("(" + to_string(t.first)) + ", ") + to_string(t.second)) + ")");
}

func to_string(t: dynamic, x1: dynamic, x2: dynamic, C: dynamic...)
{
  var ret = "[";
  x1 = min(x1, SIZE(t));
  var e = begin(t);
  advance(e, x1);
  {
    var i = x1;
    var i = min(x2, (SIZE(t) - 1));
    while ((i <= i))
    {
      ret += (to_string((*e), cpp_expand(C)) + (if ((i != i)) ", " else ""));
      e = next(e);
      i += 1;
    }
  }
  return (ret + "]");
}

class print_tuple
{
  func operator_call(t: dynamic)
  {
      var ret = [](t);
      ret += (if (Index) ", " else "");
      return (ret + to_string(get(t)));
    }
}

class print_tuple_0_Ts
{
  func operator_call(t: dynamic)
  {
      return to_string(get(t));
    }
}

func to_string(t: dynamic)
{
  var Size = tuple_size.value;
  return [](t);
}

func dbgr()
{
}

func dbgr(H: dynamic, T: dynamic...)
{
  write(to_string(H), " | ");
  dbgr(cpp_expand(T));
}

func dbgs()
{
}

func dbgs(H: dynamic, T: dynamic...)
{
  write(H, " ");
  dbgs(cpp_expand(T));
}

var MOD = 1000000007;

func output_vector(v: dynamic, line_break: dynamic = false, add_one: dynamic = false, start: dynamic = -1, end: dynamic = -1)
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
    var i = start;
    while ((i < end))
    {
      write((v[i] + (if (add_one) 1 else 0)), (if (line_break) cpp_char("\n") else if ((i < (end - 1))) cpp_char(" ") else cpp_char("\n")));
      i += 1;
    }
  }
}

func solve()
{
  var n: dynamic;
  var m: dynamic;
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
      var t = INF;
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
  var ans = INF;
  {
    int_cpp(i) = (0);
    while (((i) < (m)))
    {
      var t = 0;
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  solve();
  return 0;
}
