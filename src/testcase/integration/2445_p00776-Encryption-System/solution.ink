// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

var s: dynamic;

var ans: dynamic;

func rec(idx: dynamic, state: dynamic, sum: dynamic)
{
  if ((idx == s.size()))
  {
    ans.push_back(sum);
    return;
  }
  var c = s[idx];
  if (state[(c - cpp_char("a"))])
  {
    rec((idx + 1), state, (sum + c));
  }
  if ((((c + 1) <= cpp_char("z")) && (!state[((c + 1) - cpp_char("a"))])))
  {
    state[((c + 1) - cpp_char("a"))] = true;
    rec((idx + 1), state, (sum + string_cpp(1, ((c + 1)))));
  }
}

func main()
{
  while (((cin >> s) && (s != "#")))
  {
    ans.clear();
    var state = cpp_construct(26, false);
    state[0] = true;
    rec(0, state, "");
    write(ans.size(), "\n");
    sort(ans.begin(), ans.end());
    if ((ans.size() <= 10))
    {
      ((REP(i, ans.size()) << ans[i]) << endl);
    } else
    {
      ((REP(i, 5) << ans[i]) << endl);
      ((REP(i, 5) << ans[((ans.size() - 5) + i)]) << endl);
    }
  }
}
