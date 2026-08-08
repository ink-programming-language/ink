// Translated from solution.cpp.

func rep(i: dynamic, N: dynamic)
{
  cpp_macro("for(int i = 0; i < (N); i++)");
}

func reps(i: dynamic, N: dynamic)
{
  cpp_macro("for(int i = 1; i <= (N); i++)");
}

func repr(i: dynamic, N: dynamic)
{
  cpp_macro("for(int i = (N) - 1; i >= 0; i--)");
}

var pub = cpp_expression("#include");

func chmax(a: dynamic, b: dynamic)
{
  a = max(a, b);
}

func __cpp_top_level_1()
{
}

func chmin(a: dynamic, b: dynamic)
{
  a = min(a, b);
}

func __cpp_top_level_2()
{
}

var INF = 100000000;

var LINF = 10000000000000000;

var MOD = 1000000007;

var dx = [0, 1, 0, -1, 1, 1, -1, -1, 0];

var dy = [1, 0, -1, 0, 1, -1, -1, 1, 0];

var N: dynamic;

var D = cpp_array(51);

var b = cpp_array(24);

func solve()
{
  sort(D, (D + N));
  var ans = INF;
  var c = INF;
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

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  read(N);
  N += 1;
  solve();
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if ((i % 2))
    {
      b[D[i]] += 1;
    } else
    {
      b[(((24 - D[i])) % 24)] += 1;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    read(D[i]);
  }
