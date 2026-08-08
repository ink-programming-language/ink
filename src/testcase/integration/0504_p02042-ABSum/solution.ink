// Translated from solution.cpp.

var F = cpp_expression("#incl");

var S = cpp_expression("#inclu");

var E = (1e18 + 7);

var MOD = 1000000007;

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var Q: dynamic;
  var Q2: dynamic;
  var ans = 0;
  {
    var i = 0;
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
