// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(ll i=(a);i<(b);i++)");
}

func REP(i: dynamic, a: dynamic)
{
  return cpp_expression("#include<b");
}

var N = 400;

var S: dynamic;

func main()
{
  cin.tie(0);
  cin.sync_with_stdio(false);
  var cnt: dynamic;
  read(N, S);
  var ans = 0;
  ans = ((cnt[cpp_char("R")] * cnt[cpp_char("G")]) * cnt[cpp_char("B")]);
  write(ans, "\n");
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    cnt[S[i]] += 1;
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    FOR(j, (i + 1), N);
    {
      var k = ((2 * j) - i);
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
