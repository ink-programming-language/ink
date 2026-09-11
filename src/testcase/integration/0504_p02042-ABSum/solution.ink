// Translated from solution.cpp.

var F: dynamic = cpp_expression("#incl");

var S: dynamic = cpp_expression("#inclu");

var E: dynamic = (1e18 + 7);

var MOD: dynamic = 1000000007;

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var Q: dynamic = cpp_uninitialized();
  var Q2: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      ans += abs((a[i] - i));
      Q.push(min(a[i], i));
      Q2.push(max(a[i], i));
      i += 1;
    }
  }
  while (cpp_update(m, "--"))
  {
    ans += max((((Q.top() - Q2.top())) << 1), 0);
    Q.pop();
    Q2.pop();
  }
  write(ans, "\n");
  return 0;
}
