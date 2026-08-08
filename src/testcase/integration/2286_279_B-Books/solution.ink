// Translated from solution.cpp.

func SIZE(t: dynamic)
{
  return N;
}

func SIZE(t: dynamic)
{
  return t.size();
}

func to_string(s: dynamic, x1: dynamic = 0, x2: dynamic = 1e9)
{
  return ((cpp_char("\"") + (if (((x1 < s.size()))) s.substr(x1, ((x2 - x1) + 1)) else "")) + cpp_char("\""));
}

func to_string(s: dynamic)
{
  return to_string(cpp_cast(s));
}

func to_string(b: dynamic)
{
  return (if (b) "true" else "false");
}

func to_string(c: dynamic)
{
  return string_cpp([c]);
}

func to_string(b: dynamic, x1: dynamic = 0, x2: dynamic = 1e9)
{
  var t = "";
  {
    var iii = min(x1, SIZE(b));
    var jjj = min(x2, (SIZE(b) - 1));
    while ((iii <= jjj))
    {
      t += (b[iii] + cpp_char("0"));
      iii += 1;
    }
  }
  return ((cpp_char("\"") + t) + cpp_char("\""));
}

var l_v_l_v_l = 0;

var t_a_b_s = 0;

func to_string(p: dynamic)
{
  l_v_l_v_l += 1;
  var res = (((("(" + to_string(p.first)) + ", ") + to_string(p.second)) + ")");
  l_v_l_v_l -= 1;
  return res;
}

func to_string(v: dynamic, x1: dynamic, x2: dynamic, coords: dynamic...)
{
  var rnk = rank.value;
  var tab = cpp_construct(t_a_b_s, cpp_char(" "));
  var res = "";
  var first = true;
  if ((l_v_l_v_l == 0))
  {
    res += cpp_char("\n");
  }
  res += (tab + "[");
  x1 = min(x1, SIZE(v));
  x2 = min(x2, SIZE(v));
  var l = begin(v);
  advance(l, x1);
  var r = l;
  advance(r, (((x2 - x1)) + ((x2 < SIZE(v)))));
  {
    var e = l;
    while ((e != r))
    {
      if ((!first))
      {
        res += ", ";
      }
      first = false;
      l_v_l_v_l += 1;
      if ((e != l))
      {
        if ((rnk > 1))
        {
          res += cpp_char("\n");
          t_a_b_s = l_v_l_v_l;
        }
      } else
      {
        t_a_b_s = 0;
      }
      res += to_string((*e), cpp_expand(coords));
      l_v_l_v_l -= 1;
      e = next(e);
    }
  }
  res += "]";
  if ((l_v_l_v_l == 0))
  {
    res += cpp_char("\n");
  }
  return res;
}

func dbgm()
{
}

func dbgm(H: dynamic, T: dynamic...)
{
  write(to_string(H), " | ");
  dbgm(cpp_expand(T));
}

func solve()
{
  var n: dynamic;
  var time: dynamic;
  read(n, time);
  var arr = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  var k = 0;
  var sum = 0;
  var count = 0;
  {
    var i = 0;
    while ((i < n))
    {
      sum = (sum + arr[i]);
      if ((sum <= time))
      {
        count += 1;
      } else
      {
        sum = (sum - arr[k]);
        k += 1;
      }
      i += 1;
    }
  }
  write(count, cpp_char("\n"));
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.precision(20);
  var cases: dynamic;
  cases = 1;
  while (cpp_update(cases, "--"))
  {
    solve();
  }
}
