// Translated from solution.cpp.

func SIZE(t: dynamic) -> dynamic
{
  return N;
}

func SIZE(t: dynamic) -> dynamic
{
  return t.size();
}

func to_string(s: dynamic, x1: dynamic = 0, x2: dynamic = 1e9) -> dynamic
{
  return ((cpp_char("\"") + ( (((x1 < s.size()))) ? s.substr(x1, ((x2 - x1) + 1)) : "")) + cpp_char("\""));
}

func to_string(s: dynamic) -> dynamic
{
  return to_string(cpp_cast(s));
}

func to_string(b: dynamic) -> dynamic
{
  return ( (b) ? "true" : "false");
}

func to_string(c: dynamic) -> dynamic
{
  return string_cpp([c]);
}

func to_string(b: dynamic, x1: dynamic = 0, x2: dynamic = 1e9) -> dynamic
{
  var t: dynamic = "";
  {
    var iii: dynamic = min(x1, SIZE(b));
    var jjj: dynamic = min(x2, (SIZE(b) - 1));
    while ((iii <= jjj))
    {
      t += (b[iii] + cpp_char("0"));
      iii += 1;
    }
  }
  return ((cpp_char("\"") + t) + cpp_char("\""));
}

var l_v_l_v_l: dynamic = 0;

var t_a_b_s: dynamic = 0;

func to_string(p: dynamic) -> dynamic
{
  l_v_l_v_l += 1;
  var res: dynamic = (((("(" + to_string(p.first)) + ", ") + to_string(p.second)) + ")");
  l_v_l_v_l -= 1;
  return res;
}

func to_string(v: dynamic, x1: dynamic, x2: dynamic, coords: dynamic...) -> dynamic
{
  var rnk: dynamic = rank.value;
  var tab: dynamic = cpp_construct(t_a_b_s, cpp_char(" "));
  var res: dynamic = "";
  var first: dynamic = true;
  if ((l_v_l_v_l == 0))
  {
    res += cpp_char("\n");
  }
  res += (tab + "[");
  x1 = min(x1, SIZE(v));
  x2 = min(x2, SIZE(v));
  var l: dynamic = begin(v);
  advance(l, x1);
  var r: dynamic = l;
  advance(r, (((x2 - x1)) + ((x2 < SIZE(v)))));
  {
    var e: dynamic = l;
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

func dbgm() -> dynamic
{
}

func dbgm(H: dynamic, T: dynamic...) -> dynamic
{
  write(to_string(H), " | ");
  dbgm(cpp_expand(T));
}

var flag: dynamic = true;

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

func can() -> dynamic
{
  if ((n <= k))
  {
    return 1;
  }
  if ((k == 1))
  {
    if ((n == 1))
    {
      return 1;
    } else
    {
      return 0;
    }
  }
  var t: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i <= d))
    {
      if ((n <= t))
      {
        return 1;
      }
      t = (t * k);
      i += 1;
    }
  }
  return 0;
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, k, d);
  if ((!can()))
  {
    flag = false;
  }
  if (flag)
  {
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var nw: dynamic = (i + 1);
        {
          var j: dynamic = 0;
          while ((j < d))
          {
            ans[j][i] = (nw % k);
            nw /= k;
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < d))
      {
        {
          var j: dynamic = 0;
          while ((j < n))
          {
            write((ans[i][j] + 1), " ");
            j += 1;
          }
        }
        write("\n");
        i += 1;
      }
    }
  } else
  {
    write(-1, "\n");
  }
  return 0;
}
