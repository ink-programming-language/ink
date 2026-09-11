// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var arr: dynamic = cpp_array(n);
  var mini: dynamic = LONG_MAX;
  var flag: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  sort(arr, (arr + n));
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var val: dynamic = (arr[i] - arr[0]);
      if (((val % k) != 0))
      {
        write(-1);
        return 0;
      }
      ans += (val / k);
      i += 1;
    }
  }
  write(ans);
}
