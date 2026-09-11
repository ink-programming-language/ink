// Translated from solution.cpp.

var time: dynamic = cpp_expression("#include <bits/stdc++.h> using namespace std ; #define time cerr<<\"t");

var fast: dynamic = cpp_expression("#include <bits/stdc++.h> using na");

var MOD: dynamic = 100000007;

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return (((a * b)) / gcd(a, b));
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  var mini: dynamic = (*min_element(arr.begin(), arr.end()));
  var same: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((arr[i] == mini))
      {
        same += 1;
      }
      i += 1;
    }
  }
  write((n - same), "\n");
}

func main() -> dynamic
{
  fast;
  time;
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
