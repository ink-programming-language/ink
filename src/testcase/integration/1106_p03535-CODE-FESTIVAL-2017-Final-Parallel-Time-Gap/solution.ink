// Translated from solution.cpp.

func rep(i: dynamic, N: dynamic) -> dynamic
{
  cpp_macro("for(int i = 0; i < (N); i++)");
}

func reps(i: dynamic, N: dynamic) -> dynamic
{
  cpp_macro("for(int i = 1; i <= (N); i++)");
}

func repr(i: dynamic, N: dynamic) -> dynamic
{
  cpp_macro("for(int i = (N) - 1; i >= 0; i--)");
}

var pub: dynamic = cpp_expression("#include");

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  a = max(a, b);
}

func __cpp_top_level_1() -> dynamic
{
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  a = min(a, b);
}

func __cpp_top_level_2() -> dynamic
{
}

var INF: dynamic = 100000000;

var LINF: dynamic = 10000000000000000;

var MOD: dynamic = 1000000007;

var dx: dynamic = [0, 1, 0, -1, 1, 1, -1, -1, 0];

var dy: dynamic = [1, 0, -1, 0, 1, -1, -1, 1, 0];

var N: dynamic = cpp_uninitialized();

var D: dynamic = cpp_array(51);

var b: dynamic = cpp_array(24);

func solve() -> dynamic
{
  sort(D, (D + N));
  var ans: dynamic = INF;
  var c: dynamic = INF;
  rep(i, 24);
  {
    if ((b[i] > 1))
    {
      write(0, "\n");
      return;
    } else if (b[i])
    {
      chmin(ans, c);
      c = 1;
    } else
    {
      c += 1;
    }
  }
  write(ans, "\n");
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  read(N);
  N += 1;
  solve();
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    if ((i % 2))
    {
      b[D[i]] += 1;
    } else
    {
      b[(((24 - D[i])) % 24)] += 1;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    read(D[i]);
  }
