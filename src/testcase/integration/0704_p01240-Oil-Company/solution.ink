// Translated from solution.cpp.

func FOR(i: dynamic, k: dynamic, n: dynamic)
{
  cpp_macro("for(int i=(k); i<(int)(n); ++i)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

func FORIT(i: dynamic, c: dynamic)
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();++i)");
}

func debug(begin: dynamic, end: dynamic)
{
  {
    var i = begin;
    while ((i != end))
    {
      write((*i), " ");
      i += 1;
    }
  }
  write("\n");
}

func valid(x: dynamic, y: dynamic, W: dynamic, H: dynamic)
{
  return (((((x >= 0) && (y >= 0)) && (x < W)) && (y < H)));
}

var INF = 100000000;

var EPS = 1e-8;

var MOD = 1000000007;

var dx = [1, 0, -1, 0, 1, -1, -1, 1];

var dy = [0, 1, 0, -1, 1, 1, -1, -1];

func main()
{
  var T: dynamic;
  read(T);
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
        if (((x + y) == i))
        {
          v.push_back(r[y][x]);
        }
      }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    printf("Case %d: ", (casenum + 1));
    var W: dynamic;
    var H: dynamic;
    read(W, H);
    var r = cpp_array(20, 20);
    if ((H <= W))
    {
      cpp_statement("REP(y, H) REP(x, W)");
      read(r[y][x]);
    } else
    {
      swap(H, W);
      REP(x, W);
      REP(y, H);
      read(r[y][x]);
    }
    var dp = [];
    memset(dp, 0, cpp_sizeof((dp)));
    dp[0][0] = 0;
    dp[0][1] = r[0][0];
    var prevN = 1;
    FOR(i, 1, ((W + H) - 1));
    {
      var v: dynamic;
      REP(y, H);
      var N = v.size();
      var sum = cpp_construct((1 << N), 0);
      {
        var S = 0;
        while ((S < (1 << N)))
        {
          {
            var i = 0;
            while ((i < N))
            {
              if (((S >> i) & 1))
              {
                sum[S] += v[i];
              }
              i += 1;
            }
          }
          S += 1;
        }
      }
      {
        var S = 0;
        while ((S < (1 << N)))
        {
          if ((prevN <= N))
          {
            var mask = ((((1 << prevN)) - 1));
            var S0 = (S & mask);
            var S1 = (((S >> 1)) & mask);
            var PS = ((~((S0 | S1))) & mask);
            dp[(i & 1)][S] = max(dp[(i & 1)][S], (dp[(((i - 1)) & 1)][PS] + sum[S]));
          } else
          {
            var mask = ((((1 << prevN)) - 1));
            var S0 = (S & mask);
            var S1 = (((S << 1)) & mask);
            var PS = ((~((S0 | S1))) & mask);
            dp[(i & 1)][S] = max(dp[(i & 1)][S], (dp[(((i - 1)) & 1)][PS] + sum[S]));
          }
          S += 1;
        }
      }
      REP(j, N);
      REP(S, (1 << N));
      {
        dp[(i & 1)][(S | (1 << j))] = max(dp[(i & 1)][(S | (1 << j))], dp[(i & 1)][S]);
      }
      memset(dp[(((i - 1)) & 1)], 0, cpp_sizeof((dp[(((i - 1)) & 1)])));
      prevN = N;
    }
    write(max(dp[((((W + H) - 2)) & 1)][0], dp[((((W + H) - 2)) & 1)][1]), "\n");
  }
