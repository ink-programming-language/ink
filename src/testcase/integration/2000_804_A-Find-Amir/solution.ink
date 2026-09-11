// Translated from solution.cpp.

class Solution
{
  func solve(in_cpp: dynamic, out: dynamic) -> dynamic
  {
      var n: dynamic = cpp_uninitialized();
      (in_cpp >> n);
      var ans: dynamic = (((n / 2) + (n % 2)) - 1);
      ((out << ans) << "\n");
    }
}

func solve(in_cpp: dynamic, out: dynamic) -> dynamic
{
  (out << setprecision(12));
  var solution: dynamic = cpp_uninitialized();
  solution.solve(in_cpp, out);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var in_cpp: dynamic = cin;
  var out: dynamic = cout;
  solve(in_cpp, out);
  return 0;
}
