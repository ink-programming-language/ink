// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var arr = cpp_array(n);
  var mini = LONG_MAX;
  var flag = 0;
  {
    var i = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  sort(arr, (arr + n));
  var ans = 0;
  {
    var i = 1;
    while ((i < n))
    {
      var val = (arr[i] - arr[0]);
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
