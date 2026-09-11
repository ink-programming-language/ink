// Translated from solution.cpp.

var MOD: dynamic = (1e9 + 7);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var m: dynamic = cpp_uninitialized();
  var ans: dynamic = [];
  var sum: dynamic = [];
  m[0] += 1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      read(a);
      sum += a;
      m[sum] += 1;
      i += 1;
    }
  }
  for (var i: dynamic in m)
  {
    ans += ((i.second * ((i.second - 1))) / 2);
  }
  write(ans, "\n");
}
