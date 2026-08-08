// Translated from solution.cpp.

func main()
{
  var n = 0;
  read(n);
  var arr = cpp_construct(n, 0);
  {
    var i = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  sort(arr.begin(), arr.end(), greater());
  var ans = cpp_construct(n, 0);
  var k = 0;
  var i = (n / 2);
  var j = (i - 1);
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
    var i = 0;
    while ((i < n))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  return 0;
}
