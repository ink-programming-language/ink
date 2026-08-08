// Translated from solution.cpp.

class Solution
{
  func solve(in_cpp: dynamic, out: dynamic)
  {
      var n: dynamic;
      (in_cpp >> n);
      var ans = (((n / 2) + (n % 2)) - 1);
      ((out << ans) << "\n");
    }
}

func solve(in_cpp: dynamic, out: dynamic)
{
  (out << setprecision(12));
  var solution: dynamic;
  solution.solve(in_cpp, out);
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var in_cpp = cin;
  var out = cout;
  solve(in_cpp, out);
  return 0;
}
