// Translated from solution.cpp.

var LIM: dynamic = 26;

var INF: dynamic = 1e+9;

class str
{
  var a: dynamic = cpp_array(LIM);
  func str(s: dynamic) -> dynamic
  {
      fill(a, (a + LIM), 0);
      {
        var i: dynamic = 0;
        while ((i < cpp_cast(s.length())))
        {
          a[(s[i] - cpp_char("a"))] += 1;
          i += 1;
        }
      }
    }
}

var sa: dynamic = cpp_uninitialized();

var sb: dynamic = cpp_uninitialized();

var sc: dynamic = cpp_uninitialized();

func check(a: dynamic, cur: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  var ans: dynamic = INF;
  {
    var i: dynamic = 0;
    while ((i < LIM))
    {
      if ((a.a[i] < (b.a[i] * cur)))
      {
        ans = -1;
      } else if ((c.a[i] != 0))
      {
        ans = min(ans, (((a.a[i] - (cur * b.a[i]))) / c.a[i]));
      }
      i += 1;
    }
  }
  if ((ans != -1))
  {
    return (ans + cur);
  }
  return ans;
}

func print(a: dynamic, cur: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < cur))
    {
      write(sb);
      i += 1;
    }
  }
  var sec: dynamic = (check(a, cur, b, c) - cur);
  {
    var i: dynamic = 0;
    while ((i < sec))
    {
      write(sc);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < LIM))
    {
      a.a[i] -= (b.a[i] * cur);
      a.a[i] -= (c.a[i] * sec);
      {
        var j: dynamic = 0;
        while ((j < a.a[i]))
        {
          write(cpp_cast(((cpp_char("a") + i))));
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  read(sa, sb, sc);
  var ans: dynamic = -1;
  var step: dynamic = -1;
  var cur: dynamic = 0;
  while (true)
  {
    var cur_a: dynamic = check(a, cur, b, c);
    if ((cur_a == -1))
    {
      break;
    }
    if (((ans == -1) || (ans < cur_a)))
    {
      ans = cur_a;
      step = cur;
    }
    cur += 1;
  }
  print(a, step, b, c);
  return 0;
}
