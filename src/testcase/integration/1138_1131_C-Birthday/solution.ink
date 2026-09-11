// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = 0;
  read(n);
  var arr: dynamic = cpp_construct(n, 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  sort(arr.begin(), arr.end(), greater());
  var ans: dynamic = cpp_construct(n, 0);
  var k: dynamic = 0;
  var i: dynamic = (n / 2);
  var j: dynamic = (i - 1);
  while ((j >= 0))
  {
    if ((n > i))
    {
      ans[cpp_update(i, "++")] = arr[k];
    }
    k += 1;
    ans[cpp_update(j, "--")] = arr[k];
    k += 1;
  }
  if (((n % 2) != 0))
  {
    ans[(n - 1)] = arr[(n - 1)];
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  return 0;
}
