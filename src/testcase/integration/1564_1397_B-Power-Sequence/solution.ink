// Translated from solution.cpp.

func solve(a: dynamic, n: dynamic) -> dynamic
{
  var big: dynamic = cpp_cast(1e15);
  var ans: dynamic = big;
  var c: dynamic = 1;
  var k: dynamic = 0;
  while (1)
  {
    var val: dynamic = 0;
    var pw: dynamic = 1;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((pw >= big))
        {
          val = -1;
          break;
        }
        val += abs((pw - a[i]));
        i += 1;
        pw *= c;
      }
    }
    if ((val == -1))
    {
      break;
    }
    ans = min(ans, val);
    c += 1;
  }
  write(ans);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
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
  sort(arr.begin(), arr.end());
  solve(arr, n);
  return 0;
}
