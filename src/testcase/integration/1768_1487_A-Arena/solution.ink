// Translated from solution.cpp.

var time = cpp_expression("#include <bits/stdc++.h> using namespace std ; #define time cerr<<\"t");

var fast = cpp_expression("#include <bits/stdc++.h> using na");

var MOD = 100000007;

func gcd(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic)
{
  return (((a * b)) / gcd(a, b));
}

func solve()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  var mini = (*min_element(arr.begin(), arr.end()));
  var same = 0;
  {
    var i = 0;
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

func main()
{
  fast;
  time;
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
