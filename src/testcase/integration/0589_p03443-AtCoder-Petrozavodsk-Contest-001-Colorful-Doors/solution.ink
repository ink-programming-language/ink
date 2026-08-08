// Translated from solution.cpp.

var int_cpp = cpp_expression("#i");

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=int(a);i<int(b);i++)");
}

func REP(i: dynamic, b: dynamic)
{
  return cpp_expression("#include <");
}

var MP = cpp_expression("#include");

var PB = cpp_expression("#include");

func ALL(x: dynamic)
{
  return cpp_expression("#include <bits/st");
}

var REACH = cpp_expression("#include <bits/stdc++.h> using namesp");

func DMP(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std;");
}

func ZERO(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  (((((os << "(") << p.first) << ",") << p.second) << ")");
  return os;
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  (os << "[");
  REP(i, cpp_cast(v.size()));
  {
    if (i)
    {
      (os << ",");
    }
    (os << v[i]);
  }
  (os << "]");
  return os;
}

func read()
{
  var i: dynamic;
  scanf(cpp_expression("\"%\""), SCNd64, (&i));
  return i;
}

func printSpace()
{
  printf(" ");
}

func printEoln()
{
  printf("\n");
}

func print(x: dynamic, suc: dynamic = 1)
{
  printf(cpp_expression("\"%\""), PRId64, x);
  if ((suc == 1))
  {
    printEoln();
  }
  if ((suc == 2))
  {
    printSpace();
  }
}

func readString()
{
  var buf = cpp_array(3341000);
  scanf("%s", buf);
  return string_cpp(buf);
}

func readCharArray()
{
  var buf = cpp_array(3341000);
  var bufUsed = 0;
  var ret = (buf + bufUsed);
  scanf("%s", ret);
  bufUsed += (strlen(ret) + 1);
  return ret;
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
}

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func Sq(t: dynamic)
{
  return (t * t);
}

var inf = (LLONG_MAX / 3);

var Nmax = 200010;

var match_cpp = cpp_array(Nmax);

var vis = cpp_array(Nmax);

func Go(n: dynamic, s: dynamic)
{
  ZERO(vis);
  var pos = 0;
  while ((pos < (n * 2)))
  {
    pos = match_cpp[pos];
    vis[pos] = true;
    pos += 1;
  }
  REP(i, ((n * 2) - 1));
  if ((((s[i] == cpp_char("1"))) ^ vis[i]))
  {
    return false;
  }
  return true;
}

func Match(a: dynamic, b: dynamic)
{
  match_cpp[a] = b;
  match_cpp[b] = a;
}

func Muri()
{
  write("No", "\n");
  exit(0);
}

var col = cpp_array(Nmax);

func ShowMatch(n: dynamic)
{
  write("Yes", "\n");
  REP(i, (n * 2))[i] = -1;
  var k = 0;
  REP(i, (n * 2));
  {
    if ((col[i] == -1))
    {
      col[i] = cpp_update(k, "++");
    }
    col[match_cpp[i]] = col[i];
    print(col[i], if ((i == ((n * 2) - 1))) 1 else 2);
  }
}

func Calc(n: dynamic, s: dynamic)
{
  s = ((string_cpp("1") + s) + string_cpp("1"));
  REP(i, (n * 2))[i] = -1;
  var pos00: dynamic;
  var pos01: dynamic;
  var pos10: dynamic;
  var pos11: dynamic;
  REP(i, (n * 2));
  {
    if ((s[i] == cpp_char("0")))
    {
      if ((s[(i + 1)] == cpp_char("0")))
      {
        pos00.PB(i);
      } else
      {
        pos01.PB(i);
      }
    } else if ((s[i] == cpp_char("1")))
    {
      if ((s[(i + 1)] == cpp_char("0")))
      {
        pos10.PB(i);
      } else
      {
        pos11.PB(i);
      }
    }
  }
  if (((int_cpp(pos00.size()) % 2) == 1))
  {
    return false;
  }
  {
    var i = 0;
    while ((i < int_cpp(pos00.size())))
    {
      Match(pos00[i], pos00[(i + 1)]);
      i += 2;
    }
  }
  if (((int_cpp(pos11.size()) % 4) == 0))
  {
  } else
  {
    var ss: dynamic;
    var cur = 0;
    REP(i, ((n * 2) + 1));
    if ((s[i] == cpp_char("1")))
    {
      cur += 1;
      if (((i == (n * 2)) || (s[(i + 1)] == cpp_char("0"))))
      {
        ss.PB(cur);
      }
    } else
    {
      cur = 0;
    }
    if ((int_cpp(ss.size()) == 1))
    {
      return false;
    }
    var cnt1 = 0;
    var cnt2 = 0;
    FOR(i, 1, (int_cpp(ss.size()) - 1));
    if ((ss[i] > 1))
    {
      cnt1 += 1;
    }
    if ((ss[0] > 1))
    {
      cnt2 += 1;
    }
    if ((ss.back() > 1))
    {
      cnt2 += 1;
    }
    if (((cnt1 == 0) || ((cnt1 + cnt2) <= 1)))
    {
      return false;
    }
    var z: dynamic;
    {
      var w: dynamic;
      var last = (pos11.front() - 1);
      for (var p in pos11)
      {
        if (((last + 1) < p))
        {
          z.PB(w);
          w.clear();
        }
        last = p;
        w.PB(p);
      }
      z.PB(w);
    }
    {
      var zs = z.size();
      var a: dynamic;
      var b: dynamic;
      REP(i, zs);
      if (((1 <= z[i].front()) && (z[i].back() <= ((n * 2) - 2))))
      {
        a = i;
      }
      REP(i, zs);
      if ((a != i))
      {
        b = i;
      }
      Match((z[a].front() - 1), (z[a].back() + 1));
      pos01.erase(find(ALL(pos01), (z[a].front() - 1)));
      pos10.erase(find(ALL(pos10), (z[a].back() + 1)));
      Match(z[a].front(), z[b].back());
      z[b].pop_back();
      FOR(i, 1, z[a].size())[b].PB(z[a][i]);
      z[a].clear();
      pos11.clear();
      for (var zz in z)
      {
        for (var zzz in zz)
        {
          pos11.PB(zzz);
        }
      }
    }
  }
  REP(i, pos01.size());
  Match(pos01[i], pos10[i]);
  assert(((int_cpp(pos11.size()) % 4) == 0));
  {
    var i = 0;
    while ((i < int_cpp(pos11.size())))
    {
      Match(pos11[i], pos11[(i + 2)]);
      Match(pos11[(i + 1)], pos11[(i + 3)]);
      i += 4;
    }
  }
  return true;
}

var Nmax = 20;

var match_cpp = cpp_array(Nmax);

var vis = cpp_array(Nmax);

func Go(n: dynamic, s: dynamic)
{
  ZERO(vis);
  var pos = 0;
  while ((pos < (n * 2)))
  {
    pos = match_cpp[pos];
    vis[pos] = true;
    pos += 1;
  }
  REP(i, ((n * 2) - 1));
  if ((((s[i] == cpp_char("1"))) ^ vis[i]))
  {
    return false;
  }
  return true;
}

func rec(n: dynamic, s: dynamic)
{
  var a = (find(match_cpp, (match_cpp + (n * 2)), -1) - match_cpp);
  if ((a == (n * 2)))
  {
    return Go(n, s);
  }
  FOR(b, (a + 1), (n * 2));
  if ((match_cpp[b] == -1))
  {
    match_cpp[a] = b;
    match_cpp[b] = a;
    if (rec(n, s))
    {
      return true;
    }
    match_cpp[a] = -1;
    match_cpp[b] = -1;
  }
  return false;
}

func Calc(n: dynamic, s: dynamic)
{
  REP(i, (n * 2))[i] = -1;
  return rec(n, s);
}

func Test(n: dynamic, s: dynamic)
{
  var s1 = Fast.Calc(n, s);
  var s2 = Slow.Calc(n, s);
  if ((s1 != s2))
  {
    write("Fail", "\n");
    write(n, "\n");
    write(s, "\n");
    write(s1, " ", s2, "\n");
    exit(1);
  }
}

class xorshift
{
  func xorshift()
  {
      a = unsigned(clock());
      b = 1145141919;
      c = 810893334;
      d = 1919334810;
      REP(i, 114);
      cpp_statement("operator()()");
    }
  func operator_call()
  {
      var w = (a ^ ((a << 11)));
      a = b;
      b = c;
      c = d;
      d = (((d ^ ((d >> 19)))) ^ ((w ^ ((w >> 8)))));
      return d;
    }
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
}

var xrand: dynamic;

func irand(k: dynamic)
{
  return (xrand() % k);
}

func range(b: dynamic, e: dynamic)
{
  return (b + irand((e - b)));
}

func Ganbaru(n: dynamic)
{
  var s = cpp_construct(((n * 2) - 1), cpp_char("0"));
  REP(i, ((n * 2) - 1))[i] = (cpp_char("0") + irand(2));
  Test(n, s);
}

func Check(n: dynamic)
{
  var s = cpp_construct(((n * 2) - 1), cpp_char("0"));
  REP(i, ((n * 2) - 1))[i] = (cpp_char("0") + irand(2));
  var ans = Fast.Calc(n, s);
  if (ans)
  {
    assert(Fast.Go(n, s));
  } else
  {
  }
}

func main()
{
  var n = read();
  var s = readString();
  var ans = Fast.Calc(n, s);
  if (ans)
  {
    assert(Fast.Go(n, s));
    Fast.ShowMatch(n);
  } else
  {
    Fast.Muri();
  }
}
