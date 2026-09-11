// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

func ALL(c: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/");
}

func RALL(c: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/st");
}

func ITR(i: dynamic, b: dynamic, e: dynamic) -> dynamic
{
  cpp_macro("for(auto i=(b);i!=(e);++i)");
}

func FORE(x: dynamic, c: dynamic) -> dynamic
{
  return cpp_expression("#include <bits");
}

func REPF(i: dynamic, a: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=a,i##len=(int)(n);i<i##len;++i)");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <b");
}

func REPR(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=(int)(n);i>=0;--i)");
}

func SZ(c: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/");
}

func CONTAIN(c: dynamic, x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc");
}

func OUTOFRANGE(y: dynamic, x: dynamic, h: dynamic, w: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.");
}

func dump() -> dynamic
{
  cpp_macro("");
}

var DX: dynamic = [0, 1, 0, -1, 1, 1, -1, -1, 0];

var DY: dynamic = [-1, 0, 1, 0, -1, 1, 1, -1, 0];

var INF: dynamic = cpp_expression("#include <bi");

var INFLL: dynamic = cpp_expression("#include <bits/stdc++.h");

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  ((ITR(i, begin(v), end(v)) << (*i)) << ( ((i == (end(v) - 1))) ? "" : " "));
  return os;
}

func operator_shift_right(is: dynamic, v: dynamic) -> dynamic
{
  ITR(i, begin(v), end(v));
  (is >> (*i));
  return is;
}

func operator_shift_right(is: dynamic, p: dynamic) -> dynamic
{
  ((is >> p.first) >> p.second);
  return is;
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((b < a))
  {
    a = b;
    return 1;
  }
  return 0;
}

class before_main_function
{
  func before_main_function() -> dynamic
  {
      cpp_statement("#undef INF");
      cpp_statement("#define INF INFLL");
      cpp_statement("#define stoi stoll");
      cin.tie(0);
      ios.sync_with_stdio(false);
      write(setprecision(15), fixed);
      cpp_statement("#define endl \"\\n\"");
    }
}

var before_main_function: dynamic = cpp_uninitialized();

class BIT
{
  var dat: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  func BIT(n: dynamic) -> dynamic
  {
      n = n;
      dat = vector(n, 0);
    }
  func add(i: dynamic, x: dynamic) -> dynamic
  {
      while ((i <= n))
      {
        dat[i] += x;
        i += (i & (-i));
      }
    }
  func sum(i: dynamic) -> dynamic
  {
      var ret: dynamic = 0;
      while ((i > 0))
      {
        ret += dat[i];
        i -= (i & (-i));
      }
      return ret;
    }
}

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  if ((a.first != b.first))
  {
    return (a.first > b.first);
  }
  return (a.second > b.second);
}

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var C: dynamic = cpp_uninitialized();
  read(N, C);
  var a: dynamic = cpp_uninitialized();
  a.reserve(100005);
  var sc: dynamic = cpp_uninitialized();
  var Q: dynamic = cpp_construct(C, vector(3));
  sort(ALL(a), cmp);
  a.erase(unique(ALL(a)), end(a));
  sc.clear();
  var getidx: dynamic = __cpp_lambda_1;
  var bit: dynamic = cpp_construct((SZ(a) + 1));
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    sc[(i + 1)] = 0;
    a.push_back([0, (-((i + 1)))]);
  }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    read(Q[i][0]);
    var c: dynamic = Q[i][0];
    if ((c == 0))
    {
      read(Q[i][(j + 1)]);
    }
    if ((c == 1))
    {
      read(Q[i][(j + 1)]);
    }
    var t: dynamic = Q[i][1];
    var p: dynamic = Q[i][2];
    if ((c == 0))
    {
      sc[t] += p;
      a.push_back([sc[t], (-t)]);
    }
  }

func __cpp_lambda_1(x: dynamic) -> dynamic
{
  return distance(begin(a), lower_bound(ALL(a), x, cmp));
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    sc[(i + 1)] = 0;
    var idx: dynamic = getidx([0, (-((i + 1)))]);
    bit.add((idx + 1), 1);
  }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var c: dynamic = Q[i][0];
    if ((c == 0))
    {
      var t: dynamic = Q[i][1];
      var p: dynamic = Q[i][2];
      var idx1: dynamic = getidx([sc[t], (-t)]);
      bit.add((idx1 + 1), -1);
      sc[t] += p;
      var idx2: dynamic = getidx([sc[t], (-t)]);
      bit.add((idx2 + 1), 1);
    } else
    {
      var m: dynamic = Q[i][1];
      var L: dynamic = -1;
      var R: dynamic = SZ(a);
      while (((R - L) > 1))
      {
        var M: dynamic = (((L + R)) / 2);
        var s: dynamic = bit.sum((M + 1));
        if ((s >= m))
        {
          R = M;
        } else
        {
          L = M;
        }
      }
      dump(SZ(a), R);
      write((a[R].second * (-1)), " ", a[R].first, "\n");
    }
  }
