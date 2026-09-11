// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(ll i=(a);i<(b);i++)");
}

func REP(i: dynamic, a: dynamic) -> dynamic
{
  return cpp_expression("#include<b");
}

var N: dynamic = 400;

var S: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  cin.tie(0);
  cin.sync_with_stdio(false);
  var cnt: dynamic = cpp_uninitialized();
  read(N, S);
  var ans: dynamic = 0;
  ans = ((cnt[cpp_char("R")] * cnt[cpp_char("G")]) * cnt[cpp_char("B")]);
  write(ans, "\n");
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    cnt[S[i]] += 1;
  }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    FOR(j, (i + 1), N);
    {
      var k: dynamic = ((2 * j) - i);
      if ((S[i] == S[j]))
      {
        continue;
      }
      if ((((k < N) && (S[k] != S[i])) && (S[k] != S[j])))
      {
        ans -= 1;
      }
    }
  }
