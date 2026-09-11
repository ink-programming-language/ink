// Translated from solution.cpp.

func times(n: dynamic, i: dynamic) -> dynamic
{
  return cpp_expression("#include <bits");
}

func rtimes(n: dynamic, i: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

func upto(f: dynamic, t: dynamic, i: dynamic) -> dynamic
{
  cpp_macro("for(int i##_to_ = (t), i = (f); i <= i##_to_; i++)");
}

func uptil(f: dynamic, t: dynamic, i: dynamic) -> dynamic
{
  cpp_macro("for(int i##_to_ = (t), i = (f); i <  i##_to_; i++)");
}

func downto(f: dynamic, t: dynamic, i: dynamic) -> dynamic
{
  cpp_macro("for(int i##_to_ = (t), i = (f); i >= i##_to_; i--)");
}

func downtil(f: dynamic, t: dynamic, i: dynamic) -> dynamic
{
  cpp_macro("for(int i##_to_ = (t), i = (f); i >  i##_to_; i--)");
}

var long: dynamic = dynamic;

var __cpp_top_level_1: dynamic = cpp_fragment("defined(EBUG) && !defined(ONLINE_JUDGE)");

var debug: dynamic = cpp_expression("#inc");

var GLIBCXX_DEBUG: dynamic = cpp_expression("#inc");

var LIBCPP_DEBUG: dynamic = cpp_expression("#");

var ln: dynamic = cpp_expression("#includ");

var debug: dynamic = cpp_expression("#incl");

var ln: dynamic = cpp_expression("#includ");

var tb: dynamic = cpp_expression("#includ");

var sp: dynamic = cpp_expression("#inclu");

class BIT
{
  var N: dynamic = cpp_uninitialized();
  var node: dynamic = cpp_array(N);
  func BIT() -> dynamic
  {
      times(N, i)[i] = 0;
    }
  func set(i: dynamic, v: dynamic) -> dynamic
  {
      add(i, (v - node[i]));
    }
  func add(i: dynamic, v: dynamic) -> dynamic
  {
      {
        i += 1;
        while ((i <= N))
        {
          node[(i - 1)] += v;
          i += (i & (-i));
        }
      }
    }
  func sum(i: dynamic) -> dynamic
  {
      var s: dynamic = 0;
      {
        i += 1;
        while (i)
        {
          s += node[(i - 1)];
          i -= (i & (-i));
        }
      }
      return s;
    }
}

var A: dynamic = cpp_array(48600);

var B: dynamic = cpp_array(48600);

var C: dynamic = cpp_array(48600);

var e0: dynamic = cpp_array(48600);

var e: dynamic = cpp_array(48600);

func main() -> dynamic
{
  if ((!debug))
  {
    cin.tie(0);
    ios.sync_with_stdio(0);
  }
  var N: dynamic = cpp_uninitialized();
  scanf("%d", (&N));
  var nn: dynamic = ((N * ((N - 1))) / 2);
  times(2, o);
  {
    times(N, i)[i] = make_pair((((C[i] - (cpp_cast(A[i]) * -1e9))) / B[i]), i);
    if (debug)
    {
      (times(N, i) << d0[i].first);
    }
    sort(d0.begin(), d0.end());
    if (debug)
    {
      (times(N, i) << d0[i].first);
    }
    times(N, i)[d0[i].second] = i;
    if (debug)
    {
      (((times(N, i) << d0[i].first) << ",") << d0[i].second);
      var ln: dynamic = cpp_uninitialized();
    }
    var l: dynamic = -1e9;
    var r: dynamic = 1e9;
    times(100, oo);
    {
      var m: dynamic = (((l + r)) / 2);
      times(N, i)[i] = [(((C[i] - (cpp_cast(A[i]) * m))) / B[i]), i];
      sort(begin(d), end(d));
      var bit: dynamic = cpp_uninitialized();
      var a: dynamic = 0;
      if ((a < (((nn + 1)) / 2)))
      {
        l = m;
      } else
      {
        r = m;
      }
    }
    write(fixed, setprecision(30), l, ( (o) ? "\n" : " "));
    swap(A, B);
  }
  return 0;
}

func times(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    scanf("%d%d%d", (&A[i]), (&B[i]), (&C[i]));
  }

func times(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var aj: dynamic = e0[d[i].second];
        a += (aj - bit.sum(aj));
        bit.add(aj, 1);
      }
