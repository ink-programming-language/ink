// Translated from solution.cpp.

var ll: dynamic = dynamic;

var INF: dynamic = cpp_expression("#include");

var MOD: dynamic = cpp_expression("#include <");

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

var MAX_N: dynamic = 100005;

var lens: dynamic = cpp_array(100005);

var bit: dynamic = cpp_array((MAX_N + 1));

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func sum(i: dynamic) -> dynamic
{
  var s: dynamic = 0;
  while ((i > 0))
  {
    s += bit[i];
    i -= (i & (-i));
  }
  return s;
}

func add(i: dynamic, x: dynamic) -> dynamic
{
  while ((i <= MAX_N))
  {
    bit[i] += x;
    i += (i & (-i));
  }
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  printf("%d\n", n);
  var rest: dynamic = n;
  {
    var i: dynamic = 2;
    while ((i < (m + 1)))
    {
      rep(j, lens[(i - 1)].size());
      {
        add(lens[(i - 1)][j].first, 1);
        add(lens[(i - 1)][j].second, -1);
        rest -= 1;
      }
      var ans: dynamic = rest;
      var it: dynamic = i;
      while ((it <= m))
      {
        ans += sum(it);
        it += i;
      }
      printf("%d\n", ans);
      i += 1;
    }
  }
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    scanf("%d%d", (&l), (&r));
    r += 1;
    lens[(r - l)].push_back(P(l, r));
  }
