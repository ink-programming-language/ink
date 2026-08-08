// Translated from solution.cpp.

var int_cpp = dynamic;

func ALL(c: dynamic)
{
  return cpp_expression("#include <bits/");
}

func RALL(c: dynamic)
{
  return cpp_expression("#include <bits/st");
}

func ITR(i: dynamic, b: dynamic, e: dynamic)
{
  cpp_macro("for(auto i=(b);i!=(e);++i)");
}

func FORE(x: dynamic, c: dynamic)
{
  return cpp_expression("#include <bits");
}

func REPF(i: dynamic, a: dynamic, n: dynamic)
{
  cpp_macro("for(int i=a,i##len=(int)(n);i<i##len;++i)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <b");
}

func REPR(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=(int)(n);i>=0;--i)");
}

func SZ(c: dynamic)
{
  return cpp_expression("#include <bits/");
}

func CONTAIN(c: dynamic, x: dynamic)
{
  return cpp_expression("#include <bits/stdc");
}

func OUTOFRANGE(y: dynamic, x: dynamic, h: dynamic, w: dynamic)
{
  return cpp_expression("#include <bits/stdc++.");
}

func dump()
{
  cpp_macro("");
}

var DX = [0, 1, 0, -1, 1, 1, -1, -1, 0];

var DY = [-1, 0, 1, 0, -1, 1, 1, -1, 0];

var INF = cpp_expression("#include <bi");

var INFLL = cpp_expression("#include <bits/stdc++.h");

func operator_shift_left(os: dynamic, v: dynamic)
{
  ((ITR(i, begin(v), end(v)) << (*i)) << (if ((i == (end(v) - 1))) "" else " "));
  return os;
}

func operator_shift_right(is: dynamic, v: dynamic)
{
  ITR(i, begin(v), end(v));
  (is >> (*i));
  return is;
}

func operator_shift_right(is: dynamic, p: dynamic)
{
  ((is >> p.first) >> p.second);
  return is;
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chmin(a: dynamic, b: dynamic)
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
  func before_main_function()
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

var before_main_function: dynamic;

class BIT
{
  var dat: dynamic;
  var n: dynamic;
  func BIT(n: dynamic)
  {
      n = n;
      dat = vector(n, 0);
    }
  func add(i: dynamic, x: dynamic)
  {
      while ((i <= n))
      {
        dat[i] += x;
        i += (i & (-i));
      }
    }
  func sum(i: dynamic)
  {
      var ret = 0;
      while ((i > 0))
      {
        ret += dat[i];
        i -= (i & (-i));
      }
      return ret;
    }
}

func cmp(a: dynamic, b: dynamic)
{
  if ((a.first != b.first))
  {
    return (a.first > b.first);
  }
  return (a.second > b.second);
}

func main()
{
  var N: dynamic;
  var C: dynamic;
  read(N, C);
  var a: dynamic;
  a.reserve(100005);
  var sc: dynamic;
  var Q = cpp_construct(C, vector(3));
  sort(ALL(a), cmp);
  a.erase(unique(ALL(a)), end(a));
  sc.clear();
  var getidx = __cpp_lambda_1;
  var bit = cpp_construct((SZ(a) + 1));
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    sc[(i + 1)] = 0;
    a.push_back([0, (-((i + 1)))]);
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    read(Q[i][0]);
    var c = Q[i][0];
    if ((c == 0))
    {
      read(Q[i][(j + 1)]);
    }
    if ((c == 1))
    {
      read(Q[i][(j + 1)]);
    }
    var t = Q[i][1];
    var p = Q[i][2];
    if ((c == 0))
    {
      sc[t] += p;
      a.push_back([sc[t], (-t)]);
    }
  }

func __cpp_lambda_1(x: dynamic)
{
  return distance(begin(a), lower_bound(ALL(a), x, cmp));
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    sc[(i + 1)] = 0;
    var idx = getidx([0, (-((i + 1)))]);
    bit.add((idx + 1), 1);
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    var c = Q[i][0];
    if ((c == 0))
    {
      var t = Q[i][1];
      var p = Q[i][2];
      var idx1 = getidx([sc[t], (-t)]);
      bit.add((idx1 + 1), -1);
      sc[t] += p;
      var idx2 = getidx([sc[t], (-t)]);
      bit.add((idx2 + 1), 1);
    } else
    {
      var m = Q[i][1];
      var L = -1;
      var R = SZ(a);
      while (((R - L) > 1))
      {
        var M = (((L + R)) / 2);
        var s = bit.sum((M + 1));
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
