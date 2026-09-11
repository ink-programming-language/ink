// Translated from solution.cpp.

func FOR(i: dynamic, k: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=(k); i<(int)(n); ++i)");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

func FORIT(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();++i)");
}

func debug(begin: dynamic, end: dynamic) -> dynamic
{
  {
    var i: dynamic = begin;
    while ((i != end))
    {
      write((*i), " ");
      i += 1;
    }
  }
  write("\n");
}

func valid(x: dynamic, y: dynamic, W: dynamic, H: dynamic) -> dynamic
{
  return (((((x >= 0) && (y >= 0)) && (x < W)) && (y < H)));
}

var INF: dynamic = 100000000;

var EPS: dynamic = 1e-8;

var MOD: dynamic = 1000000007;

var dx: dynamic = [1, 0, -1, 0, 1, -1, -1, 1];

var dy: dynamic = [0, 1, 0, -1, 1, 1, -1, -1];

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
  read(T);
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        if (((x + y) == i))
        {
          v.push_back(r[y][x]);
        }
      }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    printf("Case %d: ", (casenum + 1));
    var W: dynamic = cpp_uninitialized();
    var H: dynamic = cpp_uninitialized();
    read(W, H);
    var r: dynamic = cpp_array(20, 20);
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
    var dp: dynamic = [];
    memset(dp, 0, cpp_sizeof((dp)));
    dp[0][0] = 0;
    dp[0][1] = r[0][0];
    var prevN: dynamic = 1;
    FOR(i, 1, ((W + H) - 1));
    {
      var v: dynamic = cpp_uninitialized();
      REP(y, H);
      var N: dynamic = v.size();
      var sum: dynamic = cpp_construct((1 << N), 0);
      {
        var S: dynamic = 0;
        while ((S < (1 << N)))
        {
          {
            var i: dynamic = 0;
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
        var S: dynamic = 0;
        while ((S < (1 << N)))
        {
          if ((prevN <= N))
          {
            var mask: dynamic = ((((1 << prevN)) - 1));
            var S0: dynamic = (S & mask);
            var S1: dynamic = (((S >> 1)) & mask);
            var PS: dynamic = ((~((S0 | S1))) & mask);
            dp[(i & 1)][S] = max(dp[(i & 1)][S], (dp[(((i - 1)) & 1)][PS] + sum[S]));
          } else
          {
            var mask: dynamic = ((((1 << prevN)) - 1));
            var S0: dynamic = (S & mask);
            var S1: dynamic = (((S << 1)) & mask);
            var PS: dynamic = ((~((S0 | S1))) & mask);
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
