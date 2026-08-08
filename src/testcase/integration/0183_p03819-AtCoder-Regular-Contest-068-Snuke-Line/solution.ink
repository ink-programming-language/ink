// Translated from solution.cpp.

var ll = dynamic;

var INF = cpp_expression("#include");

var MOD = cpp_expression("#include <");

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;i++)");
}

var MAX_N = 100005;

var lens = cpp_array(100005);

var bit = cpp_array((MAX_N + 1));

var n: dynamic;

var m: dynamic;

func sum(i: dynamic)
{
  var s = 0;
  while ((i > 0))
  {
    s += bit[i];
    i -= (i & (-i));
  }
  return s;
}

func add(i: dynamic, x: dynamic)
{
  while ((i <= MAX_N))
  {
    bit[i] += x;
    i += (i & (-i));
  }
}

func main()
{
  scanf("%d%d", (&n), (&m));
  printf("%d\n", n);
  var rest = n;
  {
    var i = 2;
    while ((i < (m + 1)))
    {
      rep(j, lens[(i - 1)].size());
      {
        add(lens[(i - 1)][j].first, 1);
        add(lens[(i - 1)][j].second, -1);
        rest -= 1;
      }
      var ans = rest;
      var it = i;
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

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var l: dynamic;
    var r: dynamic;
    scanf("%d%d", (&l), (&r));
    r += 1;
    lens[(r - l)].push_back(P(l, r));
  }
